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
	struct hash_node * next;
	struct tcp_connection * data;
};

struct hash_node empty_node = {.next = NULL, .data = NULL};
static struct hash_node * connection_hash[256]={&empty_node};

//Foreward declared fucntions
void  * tcp_connection_server(void * data);
int add_conn(int newSock, struct sockaddr_in raddr);

int remove_conn(struct sockaddr_in raddr);

int ip_to_last_subnet_int (char * ip);


void error(char * msg)
{
	perror(msg);
	exit(0);
}

void tcp_init(int localListenPort, int maxPending)
{
	pthread_t conn_server;
	struct listener_info l_info = {.port=localListenPort, .maxPending = maxPending };
	pthread_create(&conn_server, NULL, tcp_connection_server, (void *) &l_info);
	
	
	
	
}

void * tcp_connection_server(void * data){
	struct listener_info l_info = *(struct listener_info *)data;
	struct sockaddr_in laddr, raddr;
	unsigned int raddrLen;
	int listenSock;

	//local 
	listenSock = socket(PF_INET, SOCK_STREAM, 0);
	if (listenSock < 0) {
		error("ERROR opening local socket");
	}
	memset(&laddr, 0 , sizeof(laddr));
	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(l_info.port);
	laddr.sin_addr.s_addr = INADDR_ANY; 
	if (bind(listenSock, (struct sockaddr*) &laddr, sizeof(laddr)) < 0)
	{
		error("ERROR on binding local socket");
	}
	
	
	if (listen(listenSock, l_info.maxPending) != 0){
		error("ERROR while establishing listener\n");
	}
	
	
	for (;;){
		memset(&raddr, 0 , sizeof(raddr));
		raddrLen = sizeof(raddr);
		int newSock = accept(listenSock, (struct sockaddr *)&raddr, &raddrLen);
		add_conn(newSock, raddr);
	}

}

void tcp_close(char * ip, int port){

}

void tcp_close_all(){


}

int remove_conn(struct sockaddr_in raddr){
	struct hash_node * temp;
	
	int i = ip_to_last_subnet_int(inet_ntoa(raddr.sin_addr));
	int port = ntohs(raddr.sin_port);
	temp = connection_hash[i];
	while (temp->next != NULL){
		if (ntohs(temp->next->data->addr.sin_port) != port){
			
			close(temp->data->sock);
			
			return 0;
		}else{
			temp = temp->next;
		}
	}
	return -1;
}

int add_conn(int newSock, struct sockaddr_in raddr){
	// PORT int i = ntohs(raddr.sin_port);
	struct hash_node * new = (struct hash_node *) malloc(sizeof(struct hash_node));
	new->next = NULL;
	new->data = (tcp_connection){.addr = raddr, .sock = newSock};
	
	int i = ip_to_last_subnet_int(inet_ntoa(raddr.sin_addr));
	
	struct hash_node * temp = connection_hash[i];
	while (temp->next != NULL){
		temp = temp->next;
	} 
	temp->next = new;
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

