#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>

#include "common.h"

#define MAX_CL 128

struct client_info{
  int fds;
  struct sockaddr info; 
  struct client_info* next;
};


int echo_server(int sockfd) {
  int len;
  // Receiving message
  if (recv(sockfd, &len, sizeof(int), 0) <= 0) {
    printf("error receiv");
  }
  
  char* buff=malloc(len+1);
  
  if (recv(sockfd, buff,len, 0) <= 0) {
    printf("error receiv");
  }
  else{
    buff[len] = '\0';
    printf("Received: %s", buff);
  }
  
  // Sending message (ECHO)
  if (send(sockfd, &len, sizeof(int),0) <= 0) {
    printf("error sending");
  };
  
  if (send(sockfd, buff, len, 0) <= 0) {
    printf("error sending");
  }
  else{
    printf("Message sent!\n");
  }
  if(strcmp(buff,"/quit")==0){
    free(buff);
    return(1);
  }
  free(buff);
  return(0);
}

int handle_bind(char* serv_port) {
  char* s_port;
  if(serv_port !=NULL){
    s_port = serv_port;
  }
  else {
    s_port= SERV_PORT;
  }
    
  struct addrinfo hints, *result, *rp;
  int sfd;
       
        
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, s_port, &hints, &result) != 0) {
    perror("getaddrinfo()");
    exit(EXIT_FAILURE);
  }
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype,
		 rp->ai_protocol);
    int yes=1;
    setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    if (sfd == -1) {
      continue;
    }
    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;
    }
    close(sfd);
  }
  if (rp == NULL) {
    fprintf(stderr, "Could not bind\n");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(result);
  return sfd;
}

void multi(struct pollfd* fds, struct sockaddr cli, socklen_t len){
  struct client_info* client=malloc(sizeof(struct client_info));
  client->next=NULL;
  struct client_info* position=client;				    
  int connfd;
  int quit=0;
  while(1){
    int nb_act=poll(fds, MAX_CL, -1);
    if (nb_act < 0){
      perror("poll pb");
      exit(EXIT_FAILURE);
    }
    for (int i=0; i< MAX_CL; i++){
            
      if (i==0 && fds[0].revents & POLLIN){
	fds[0].revents=0;
	if ((connfd = accept(fds[0].fd, (struct sockaddr*) &cli, &len)) < 0){
	  perror("accept()\n");
	  exit(EXIT_FAILURE);
	}
	//gestion de la structure
	while (position->next!=NULL){
	  position=position->next;
	}
	struct client_info* node=malloc(sizeof(struct client_info));
	node->fds=connfd;
	node->info=cli;
	node->next=NULL;
	
	for (size_t j=0; j < MAX_CL; j++){
	  if (fds[j].fd == -1){
	    fds[j].fd = connfd;
	    fds[j].events=POLLIN;
	    fds[j].revents=0;
	    fprintf(stdout,"lol %i",connfd);
	    break;
	  }
	}
      }
      else if (i!=0 && fds[i].revents & POLLIN){
	quit=echo_server(fds[i].fd);
	fds[i].revents = 0;
	if (quit==1){
	  close(fds[i].fd);
	  fds[i].fd=-1;
	  fds[i].events=0;
	  fds[i].revents=0;
	}
      }
    }
  }

}

int main(int argc, char* argv[]) {
  struct sockaddr cli;
  int sfd;
  socklen_t len;
  int listen_sock;

  if (argc ==2){
    sfd = handle_bind(argv[1]);
  }
  else{
    sfd = handle_bind(NULL);
  }
        
  if ((listen_sock=listen(sfd, SOMAXCONN)) != 0) {
    perror("listen()\n");
    exit(EXIT_FAILURE);
  }
  len = sizeof(cli);

  struct pollfd fds[MAX_CL];
  fds[0].fd=sfd;
  fds[0].events=POLLIN;
  fds[0].revents=0;
        
        
  for (int i=1; i< MAX_CL; i++){
    fds[i].fd=-1;
    fds[i].events=0;
    fds[i].revents=0;
  }
  multi(fds,cli,len);
  close(sfd);
  return EXIT_SUCCESS;
}

