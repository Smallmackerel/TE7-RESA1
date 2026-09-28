#ifndef JALON1_COMMON_H
#define JALON1_COMMON_H

#include <stddef.h>

void die(int val, char *msg);
int read_from_socket(int fd, void *buf, size_t msg_size);
int write_in_socket(int fd, void *buf, size_t msg_size);

#endif
