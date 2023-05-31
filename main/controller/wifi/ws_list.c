#include <assert.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lwipopts.h"


static size_t fd_to_index(int fd);
static int    index_to_fd(size_t index);

static uint8_t active_ws_map[CONFIG_LWIP_MAX_SOCKETS] = {0};


int ws_list_is_active(size_t index) {
    if (index < CONFIG_LWIP_MAX_SOCKETS) {
        return active_ws_map[index];
    } else {
        return 0;
    }
}


void ws_list_activate(int fd) {
    if (fd >= LWIP_SOCKET_OFFSET) {
        active_ws_map[fd_to_index(fd)] = 1;
    }
}


int ws_list_deactivate(int fd) {
    if (fd >= LWIP_SOCKET_OFFSET) {
        if (active_ws_map[fd_to_index(fd)]) {
            active_ws_map[fd_to_index(fd)] = 0;
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}


int ws_list_get_fd(size_t index) {
    if (active_ws_map[index] && index < CONFIG_LWIP_MAX_SOCKETS) {
        return index_to_fd(index);
    } else {
        return -1;
    }
}


static size_t fd_to_index(int fd) {
    assert(fd >= LWIP_SOCKET_OFFSET);
    size_t index = fd - LWIP_SOCKET_OFFSET;
    assert(index < CONFIG_LWIP_MAX_SOCKETS);
    return index;
}


static int index_to_fd(size_t index) {
    assert(index < CONFIG_LWIP_MAX_SOCKETS);
    return index + LWIP_SOCKET_OFFSET;
}
