#include "common.h"
#include "client_list.h"
#include "msg_struct.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_MESSAGE_SIZE 4096
#define MAX_CLIENTS 128


void unicast_send(struct client_info **clients, struct message received,char* payload){
    struct client_info** cursor=clients;
  int size = received.pld_len;
  struct message msg;
  msg.pld_len = size;
  strcpy(msg.nick_sender,received.nick_sender);
  msg.type = MULTICAST_SEND;
  
  while(*cursor!=NULL){
    if (strcmp(received.nick_sender,(*cursor)->nick) ==0){ //eviter d'envoyer un message celui qui demande
      write_in_socket((*cursor)->fd, &msg, sizeof(struct message));
      write_in_socket((*cursor)->fd, payload, size);
    }
    cursor=&(*cursor)->next;
  }
}

void multicast_send(struct client_info **clients, int client_fd,struct message received,char* payload){
  struct client_info** cursor=clients;
  int size = received.pld_len;
  struct message msg;
  msg.pld_len = size;
  strcpy(msg.nick_sender,received.nick_sender);
  msg.type = MULTICAST_SEND;
  
  while(*cursor!=NULL){
    if (client_fd != (*cursor)->fd){ //eviter d'envoyer un message celui qui demande
      write_in_socket((*cursor)->fd, &msg, sizeof(struct message));
      write_in_socket((*cursor)->fd, payload, size);
    }
    cursor=&(*cursor)->next;
  }
}

void nickname_infos(struct client_info **clients, int client_fd,struct message received){
  struct client_info** cursor=clients;
  
  while(*cursor!=NULL){
    if (strcmp(received.infos,(*cursor)->nick)==0){
      struct message msg;
      struct sockaddr_in addr = (*cursor)->address;
      msg.pld_len = sizeof(char*);
      msg.type = NICKNAME_INFOS;
      write_in_socket(client_fd, &msg, sizeof(struct sockaddr_in));
      write_in_socket(client_fd, &addr , sizeof(struct sockaddr_in));
      return;
    }
    cursor=&(*cursor)->next;
  }
  //gerer la date ?????
}



void nickname_list(struct client_info **clients, int client_fd){
  struct client_info** cursor=clients;
  int size=0;
  //on recupere la taille de la liste chaine
  while(*cursor!=NULL){
    size++;
    cursor=&(*cursor)->next;
  }
  char** tab_name=malloc(size);
  int j=0;
  cursor=clients;
  //on stocke tous les noms dans le tableau
  while(*cursor!=NULL){
    if (client_fd != (*cursor)->fd){ //eviter de renvoyer le nom de celui qui demande
      strcpy(tab_name[j],(*cursor)->nick);
      j++;
    }
    cursor=&(*cursor)->next;
  }
  struct message msg;
  msg.pld_len = size*sizeof(char*);
  msg.type = NICKNAME_LIST;
  write_in_socket(client_fd, &msg, sizeof(struct message));
  write_in_socket(client_fd, tab_name, size*sizeof(char*));
  free(tab_name);
}

//ajoute/modifie un nom.
void nickname_new(struct message msg, struct client_info **clients,int client_fd){
  struct client_info** cursor=clients;
  
  
  // on verifie chaque pseudo pour voir sil nexiste pas deja
  while(*cursor!=NULL){
    if (strcmp((*cursor)->nick,msg.infos) == 0){
      char* msg_error="Pseudo déjà attribué";
      struct message msg;
      msg.pld_len = sizeof(char*);
      msg.type = NICKNAME_NEW;
      write_in_socket(client_fd, &msg, sizeof(struct message));
      write_in_socket(client_fd, msg_error, sizeof(char*)); 
      return;
    }
    cursor=&(*cursor)->next;    
  }
  //on met à jour le nom
  cursor=clients; //on retourne au debut
  while(*cursor!=NULL){
    if ((*cursor)->fd == client_fd){
      strcpy((*cursor)->nick,msg.infos);
      return;
    }
    cursor=&(*cursor)->next;    
  }
}

//fonction qui choisi l'action a realiser
void action(struct message msg, struct client_info **clients,int client_fd){
  switch (msg.type){
  case NICKNAME_NEW:
    nickname_new(msg,clients,client_fd);
    break;
  case NICKNAME_LIST:
    nickname_list(clients,client_fd);
    break;
  case NICKNAME_INFOS:
    nickname_infos(clients,client_fd,msg);
  case ECHO_SEND:
  case UNICAST_SEND:
  case BROADCAST_SEND:
  case MULTICAST_CREATE:
  case  MULTICAST_LIST:
  case  MULTICAST_JOIN:
  case  MULTICAST_SEND:
  case  MULTICAST_QUIT:
  case  FILE_REQUEST:
  case  FILE_ACCEPT:
  case  FILE_REJECT:
  case  FILE_SEND:
  case FILE_ACK:
  default:
    return;
  }
  return;
  
}


int setup_listening_socket(int port) {
	int listen_fd;
	int result;
	struct sockaddr_in server_address;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	die(listen_fd, "socket");
	printf("TCP listening socket created.\n");

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); // To listen on all interfaces --- Equivalent to 0.0.0.0
	server_address.sin_port = htons((unsigned short)port);
	result = bind(listen_fd, (struct sockaddr *)&server_address, sizeof(server_address));
	die(result, "bind");
	printf("Socket bound to port %d.\n", port);

	result = listen(listen_fd, 20);
	die(result, "listen");
	printf("Listening for client connections.\n");
	return listen_fd;
}

void accept_and_insert_client(int listen_fd, struct pollfd poll_fds[MAX_CLIENTS], struct client_info **clients) {
	struct sockaddr_in client_address;
	socklen_t client_address_length = sizeof(client_address);
	int client_fd = accept(listen_fd, (struct sockaddr *)&client_address,
		&client_address_length);
	int slot;

	die(client_fd, "accept");
	for (slot = 1; slot < MAX_CLIENTS; slot++) {
		if (poll_fds[slot].fd < 0) {
			if (client_list_add(clients, client_fd, &client_address) < 0) {
				close(client_fd);
				die(-1, "malloc client information");
			}
			poll_fds[slot].fd = client_fd;
			poll_fds[slot].events = POLLIN;
			poll_fds[slot].revents = 0;
			printf("Accepted client %s:%u on slot %d.\n",
				inet_ntoa(client_address.sin_addr),
				(unsigned int)ntohs(client_address.sin_port), slot);
			break;
		}
	}
	if (slot == MAX_CLIENTS) {
		fprintf(stderr, "Client limit reached. Closing the new connection.\n");
		close(client_fd);
	}
}

/* Return 1 when the client should be disconnected, 0 after a successful echo. */
int handle_client_message(int client_fd,struct client_info **clients) {
  struct message info;
  char message[MAX_MESSAGE_SIZE + 1];

	// first read next message size
	if (read_from_socket(client_fd, &info, sizeof(struct message)) == 0) {
		fprintf(stderr, "Client %d : Socket close\n", client_fd);
		return 1;
	}
	//regarder si le client quitte
	if (info.pld_len == -1) {
		printf("Client %d requested to quit.\n", client_fd);
		return 1;
		}
	if (info.pld_len == 0){
	  action(info,clients,client_fd);
	  return(1);
	}
	// then read the message payload
	if (read_from_socket(client_fd, message, info.pld_len) == 0) {
		fprintf(stderr, "Client %d : Socket close\n", client_fd);
		return 1;
	}

	message[info.pld_len] = '\0';
	/*
	
	if (write_in_socket(client_fd, &message_size, sizeof(message_size)) == 0 ||
		write_in_socket(client_fd, message, message_size) == 0) {
		return 1;
		}*/
	return 0;
}

void server_poll_loop(int listen_fd, struct pollfd poll_fds[MAX_CLIENTS],
		struct client_info **clients) {
	int running = 1;

	/* Slot 0 is the listener. The other slots contain client sockets. */
	for (int i = 0; i < MAX_CLIENTS; i++) {
		poll_fds[i].fd = -1;
		poll_fds[i].events = 0;
		poll_fds[i].revents = 0;
	}
	poll_fds[0].fd = listen_fd;
	poll_fds[0].events = POLLIN;

	// execute server logic
	while (running) {
		int ready = poll(poll_fds, MAX_CLIENTS, -1);
		die(ready, "poll");

		if ((poll_fds[0].revents & POLLIN) != 0) {
			accept_and_insert_client(listen_fd, poll_fds, clients);
		}

		for (int slot = 1; slot < MAX_CLIENTS; slot++) {
			short returned_events = poll_fds[slot].revents;
			int close_connection = 0;
			if (poll_fds[slot].fd < 0) {
				continue;
			}

			if ((returned_events & POLLIN) != 0) {
			  close_connection = handle_client_message(poll_fds[slot].fd,clients);
			}
			if ((returned_events & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
				close_connection = 1;
			}
			if (close_connection) {
				int client_fd = poll_fds[slot].fd;
				close(client_fd);
				client_list_remove(clients, client_fd);
				poll_fds[slot].fd = -1;
				poll_fds[slot].events = 0;
				poll_fds[slot].revents = 0;
			}
		}
		if ((poll_fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
			running = 0;
		}
	}

	// Cleaning up: close all client sockets and free the client list
	for (int slot = 1; slot < MAX_CLIENTS; slot++) {
		if (poll_fds[slot].fd >= 0) {
			close(poll_fds[slot].fd);
			poll_fds[slot].fd = -1;
		}
	}
	client_list_destroy(clients);
}

int main(int argc, char **argv) {
	struct pollfd poll_fds[MAX_CLIENTS];
	struct client_info *clients = NULL;
	int port;
	int listen_fd;

	if (argc != 2) {
		fprintf(stderr, "Usage: ./server <server_port>\n");
		return EXIT_FAILURE;
	}
	port = atoi(argv[1]);
	if (port < 1 || port > 65535) {
		fprintf(stderr, "Invalid port\n");
		return EXIT_FAILURE;
	}

	listen_fd = setup_listening_socket(port);
	server_poll_loop(listen_fd, poll_fds, &clients);
	close(listen_fd);
	return EXIT_SUCCESS;
}
