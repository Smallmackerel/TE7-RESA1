#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include "common.h"

void echo_client(int sockfd) {
    char buff[MSG_LEN];
    int n;
    //Polling
    struct pollfd fds[2];
    //STDIN
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    //Server entry
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    while(1){
        // Cleaning memory
        memset(buff, 0, MSG_LEN);
        int message_size;
        // Getting message from client
        int ret = poll(fds,2,-1);
        if(ret == -1){
            perror("Poll Error...");
            break;
        }
        printf("Message: ");
        while ((buff[n++] = getchar()) != '\n') {} // trailing '\n' will be sent
        message_size = strlen(buff); // on envoie bien strlen et pas strlen +1
        //Case of keyboard entry
        if(fds[0].revents & POLLIN){
            n = 0;
            // Sending message size
            if(send(sockfd, &message_size, sizeof(message_size), 0) <= 0){
                break;
            }
            // Sending message (ECHO)
            if (send(sockfd, buff, strlen(buff), 0) <= 0) {
                break;
                }
            printf("Message sent!\n");
        }
        //Cleaning memory
        memset(buff,0,MSG_LEN);
        if(fds[1].revents & POLLIN){
            //Receiving message size
            if(recv(sockfd, &message_size, sizeof(message_size),0) <= 0){
                break;
            }
            if (recv(sockfd, buff, message_size, 0) <= 0) {
                break;
            }
            printf("Received: %s", buff);
        }
    }
}

int handle_connect(char* serv_addr, char* serv_port, int defined) {
    char* s_addr;
    char* s_port;
    if(defined){
        s_addr = serv_addr;
        s_port = serv_port;
    }
    else{
        s_addr = SERV_ADDR;
        s_port = SERV_PORT;
    }
	struct addrinfo hints, *result, *rp;
	int sfd;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(s_addr, s_port, &hints, &result) != 0) {
		perror("getaddrinfo()");
		exit(EXIT_FAILURE);
	}
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
		if (sfd == -1) {
			continue;
		}
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}
		close(sfd);
	}
	if (rp == NULL) {
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	return sfd;
}

int main(int argc, char* argv[]) {
    int sfd;
    if(argc == 3){
	    sfd = handle_connect(argv[1], argv[2], argc == 3);
    }
    else{
	    sfd = handle_connect(NULL, NULL, 0);
    }



	echo_client(sfd);
	close(sfd);
	return EXIT_SUCCESS;
}
