#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void die(int val, char *msg) {
	if (val < 0) {
		perror(msg);
		exit(EXIT_FAILURE);
	}
}

int read_from_socket(int fd, void *buf, size_t msg_size) {
	size_t total = 0;
	char *cursor = buf;

	while (total < msg_size) {
		ssize_t n = read(fd, cursor + total, msg_size - total);
		die((int)n, "read");
		if (n == 0) {
			return 0;
		}
		total += (size_t)n;
	}
	return (int)total;
}

int write_in_socket(int fd, void *buf, size_t msg_size) {
	size_t total = 0;
	char *cursor = buf;

	while (total < msg_size) {
		ssize_t n = write(fd, cursor + total, msg_size - total);
		die((int)n, "write");
		if (n == 0) {
			return 0;
		}
		total += (size_t)n;
	}
	return (int)total;
}
