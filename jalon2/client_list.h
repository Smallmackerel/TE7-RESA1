#ifndef JALON1_CLIENT_LIST_H
#define JALON1_CLIENT_LIST_H

#define NICK_LEN 128
#include <netinet/in.h>

//deplacer la structure dans le point h pour que tout le monde y accede
struct client_info {
  int fd;
  struct sockaddr_in address;
  char nick[NICK_LEN];
  struct client_info *next;
};


struct client_info;

int client_list_add(struct client_info **clients, int fd, const struct sockaddr_in *address);
void client_list_remove(struct client_info **clients, int fd);
void client_list_destroy(struct client_info **clients);

#endif
