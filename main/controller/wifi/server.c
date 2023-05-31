#include <string.h> /* for memcpy */
#include <esp_http_server.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "server.h"
#include "ws_list.h"
#include "model/model.h"
#include "controller/rtc/rtc.h"

static esp_err_t open_cb(httpd_handle_t server, int sockfd);
static void close_cb(httpd_handle_t server, int sockfd);
static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);
static esp_err_t app_get_handler(httpd_req_t *req);
static esp_err_t rtc_ws_handler(httpd_req_t *req);
static void notify_connected_clients(void *arg);
static int server_is_anyone_connected(void);

/* web server instance */
httpd_handle_t server = NULL;
/* queue of msg received through rtc connection */
static QueueHandle_t rtc_ws_msg_queue = NULL;


static const char *TAG = "WebServer";


void webserver_init() {
    rtc_ws_msg_queue = xQueueCreate(16, sizeof(rtc_ws_msg_t));
}

httpd_handle_t start_webserver(model_t *pmodel) {
    if (server != NULL) {
        ESP_LOGW(TAG, "Web server was already started!");
        return server;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_open_sockets = 4;
    config.open_fn = open_cb;
    config.close_fn = close_cb;

    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGW(TAG, "Web server failed to start.");
        return NULL;
    }

    /* register app URI handler */
    const httpd_uri_t app_get = {
        .uri      = "/",
        .method   = HTTP_GET,
        .handler  = app_get_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &app_get);

    /* register RTC websocket URI handler */
    const httpd_uri_t rtc_ws = {
        .uri          = "/rtc",
        .method       = HTTP_GET,
        .handler      = rtc_ws_handler,
        .user_ctx     = pmodel,
        .is_websocket = true
    };
    httpd_register_uri_handler(server, &rtc_ws);

    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);

    ESP_LOGI(TAG, "Web server started on port %hu", config.server_port);
    return server;
}

esp_err_t stop_webserver(httpd_handle_t server)
{
    return httpd_stop(server);
}

static esp_err_t open_cb(httpd_handle_t server, int sockfd) {
    (void) server;

    ESP_LOGI(TAG, "Opening websocket %d", sockfd);
    return ESP_OK;
}

static void close_cb(httpd_handle_t server, int sockfd) {
    (void) server;

    ESP_LOGI(TAG, "Deactivating websocket %d", sockfd);
    ws_list_deactivate(sockfd);
    close(sockfd);
}

// HTTP Error (404) Handler - Redirects all requests to the root page
static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    // Set status
    httpd_resp_set_status(req, "302 Temporary Redirect");
    // Redirect to the "/" root directory
    httpd_resp_set_hdr(req, "Location", "/");
    // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
    httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(TAG, "Redirecting to root");
    return ESP_OK;
}

static esp_err_t app_get_handler(httpd_req_t *req) {
    extern const unsigned char app_html_start[] asm("_binary_app_html_gz_start");
    extern const unsigned char app_html_end[]   asm("_binary_app_html_gz_end");
    const size_t app_html_size = app_html_end - app_html_start;

    httpd_resp_set_hdr(req, "Content-encoding", "gzip");
    httpd_resp_send(req, (const char *) app_html_start, app_html_size);

    return ESP_OK;
}

static esp_err_t rtc_ws_handler(httpd_req_t *req) {
    esp_err_t rc;

    /* handshake, connection opened */
    if (req->method == HTTP_GET) {
        int fd = httpd_req_to_sockfd(req);
        ESP_LOGI(TAG, "New client subscribed: %i", fd);
        ws_list_activate(fd);
        rtc_send_model_replace_message((model_t *) req->user_ctx);
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;

    /* Set max_len = 0 to get the frame len */
    rc = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", rc);
        return rc;
    }
    ESP_LOGI(TAG, "Websocket frame len is %d", ws_pkt.len);

    if (ws_pkt.len > 0) {
        uint8_t *buf = malloc(ws_pkt.len);

        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        rc = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (rc != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", rc);
            free(buf);
            return rc;
        }

        // rtc_ws_msg_queue_t msg = {.data=ws_pkt.payload, .size=ws_pkt.len};
        /* remember to free buf when the msg is handled */
        rtc_ws_msg_t msg = {.data=buf, .size=ws_pkt.len};
        if (xQueueSend(rtc_ws_msg_queue, &msg, 0) != pdPASS) {
            ESP_LOGE(TAG, "xQueueSend failed on ws_rtc_handler.");
            free(buf);
            return ESP_ERR_NO_MEM;
        }
    }

    return ESP_OK;
}

int rtc_ws_get_next_msg(rtc_ws_msg_t *ws_msg) {
    return xQueueReceive(rtc_ws_msg_queue, ws_msg, 0) == pdPASS;
}

void rtc_ws_send_binary(void *msg_data, size_t msg_size) {
    if (server == NULL || !server_is_anyone_connected()) {
        return;
    }

    /* remember to free both msg and msg data when handled */
    rtc_ws_msg_t *msg = malloc(sizeof(rtc_ws_msg_t));
    msg->data = malloc(msg_size);
    memcpy(msg->data, msg_data, msg_size);
    msg->size = msg_size;

    httpd_queue_work(server, notify_connected_clients, msg);
}

static void notify_connected_clients(void *arg) {
    if (server == NULL || arg == NULL) {
        return;
    }

    rtc_ws_msg_t* msg = (rtc_ws_msg_t *) arg;

    for (size_t i = 0; i < CONFIG_LWIP_MAX_SOCKETS; i++) {
        int ws_fd = ws_list_get_fd(i);

        if (ws_fd > 0) {
            ESP_LOGD(TAG, "notifica connessione %i", ws_fd);
            httpd_ws_frame_t ws_pkt = {0};
            ws_pkt.payload          = msg->data;
            ws_pkt.len              = msg->size;
            ws_pkt.type             = HTTPD_WS_TYPE_BINARY;

            httpd_ws_send_frame_async(server, ws_fd, &ws_pkt);
        }
    }

    free(msg->data);
    free(msg);
}

static int server_is_anyone_connected(void) {
    if (server != NULL) {
        for (size_t i = 0; i < CONFIG_LWIP_MAX_SOCKETS; i++) {
            if (ws_list_is_active(i)) {
                ESP_LOGD(TAG, "Found active connection");
                return 1;
            }
        }
        ESP_LOGD(TAG, "No active connections");
        return 0;
    } else {
        return 0;
    }
}
