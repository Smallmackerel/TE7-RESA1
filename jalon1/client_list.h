#ifndef JALON1_CLIENT_LIST_H
#define JALON1_CLIENT_LIST_H

#include <netinet/in.h>

struct client_info;

int client_list_add(struct client_info **clients, int fd, const struct sockaddr_in *address);
void client_list_remove(struct client_info **clients, int fd);
void client_list_destroy(struct client_info **clients);

#endif
