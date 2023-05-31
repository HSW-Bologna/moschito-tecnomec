#ifndef WS_LIST_H_INCLUDED
#define WS_LIST_H_INCLUDED

#include <stdlib.h>


int  ws_list_is_active(size_t index);
void ws_list_activate(int fd);
int  ws_list_deactivate(int fd);
int  ws_list_get_fd(size_t index);

#endif