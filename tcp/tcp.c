#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/types.h>  /* for Socket data types */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <netinet/in.h> /* for IP Socket data types */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#include <pthread.h>

#include "tcp.h"

struct tcp_connection{
	struct sockaddr_in addr;
	int sock;
};

struct listener_info{
	int port;
	int maxPending;

};

struct hash_node {
	int * next;
	struct tcp_connection data;
};

struct hash_node* connection_hash [255] = {.[0 .. 255] = (struct hash_node*){.next = NULL, .data = (struct tcp_connection)NULL}}; 


void error(char * msg)
{
	perror(msg);
	exit(0);
}

void tcp_init(int localListenPort, int maxPending)
{
	pthread_t conn_server;
	struct listener_info l_info = {.port=localListenPort, .maxPending = maxPending };
	pthread_create(&conn_server, NULL, tcp_connection_server, (void *) l_info);
	
	
	
	
}

void tcp_connection_server(void * data){
	struct listener_info l_info = *(struct listener_info *)data;
	struct sockaddr_in laddr, raddr;
	unsigned int raddrLen;
	int listenSock;

	//local 
	lsocket = socket(PF_INET, SOCK_STREAM, 0);
	if (lsocket < 0) {
		error("ERROR opening local socket");
	}
	memset(&laddr, 0 , sizeof(laddr));
	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(l_info.localListenPort);
	laddr.sin_addr.s_addr = INADDR_ANY; 
	if (bind(lsocket, (struct sockaddr*) &laddr, sizeof(laddr)) < 0)
	{
		error("ERROR on binding local socket");
	}
	
	
	if (listen(listenSock, l_info.maxPending) != 0){
		error("ERROR while establishing listener\n");
	}
	
	
	for (;;){
		memset(&raddr, 0 , sizeof(raddr));
		raddrLen = sizeof(raddr);
		int newSock = accept(listenSock, &raddr, &raddrLen);
		add(newSock, raddr);
	}

}

void tcp_close(char * ip, int port){

}

void tcp_close_all(){


}

int remove(struct sockaddr_in raddr){
	struct hash_node * temp;
	
	int i = ip_to_last_subnet_int(inet_ntoa(raddr.sin_addr));
	int port = ntohs(raddr.sin_port);
	while (temp->next != NULL){
		if (ntohs(temp->next->data->addr) != port){
			
			close(temp->data->sock);
			
			return 0;
		}else{
			temp = temp->next;
		}
	}
	return -1;



}

void add(int newSock, struct sockaddr_in raddr){
	// PORT int i = ntohs(raddr.sin_port);
	struct hash_node * new = (struct hash_node *) malloc(sizeof(struct hash_node));
	new.next = NULL;
	new.tcp_connection = (tcp_connection){.addr = raddr, .sock = newSock};
	
	int i = ip_to_last_subnet_int(inet_ntoa(raddr.sin_addr));
	
	struct hash_node * temp = connection_hash[i];
	while (temp.next != NULL){
		temp = temp.next;
	} 
	temp.next = new;
}

int ip_to_last_subnet_int (char * ip) {
	int i;
	char * str = strdup(ip);
	char * token = strtok(str, ".");
	for (i = 0; i < 3; i++) {
		token = strtok(NULL, ".");	
	}
	return atoi(token);
}

