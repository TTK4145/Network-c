#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/types.h>  /* for Socket data types */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <netinet/in.h> /* for IP Socket data types */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#include "udp.h"

int bsocket, lsocket;


void error(char * msg)
{
	perror(msg);
	exit(0);
}

void udp_init(int localPort, int bcastPort, char * baddr_s)
{

	struct sockaddr_in laddr, baddr;
	/* broadcast */
	bsocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (bsocket < 0) {
		error("ERROR opening broadcast socket");
	}
	memset(&baddr, 0, sizeof(baddr));
	baddr.sin_family = AF_INET;
	baddr.sin_port = htons(bcastPort);
	baddr.sin_addr.s_addr = inet_addr(baddr_s);
	
	if (bind(bsocket, (struct sockaddr*) &baddr, sizeof(baddr)) < 0)
	{
		error("ERROR on binding broadcast socket");
	}
	
	
	/* local */
	lsocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (lsocket < 0) {
		error("ERROR opening local socket");
	}
	memset(&laddr, 0 , sizeof(laddr));
	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(bcastPort);
	laddr.sin_addr.s_addr = INADDR_ANY; 
	if (bind(lsocket, (struct sockaddr*) &laddr, sizeof(laddr)) < 0)
	{
		error("ERROR on binding local socket");
	}
}


int udp_sendTo(char * ip, int port, char * msg, int length){
	int n;
 	struct sockaddr_in addr;
 	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	n = sendto(lsocket, msg, sizeof(*msg), 0, (struct sockaddr*) &addr, sizeof(addr));
	if(n <= 0){
		return n;
	}
	else 
	{
		return -1;
	
	}
}



struct udp_message udp_receive_local(){
	int n;
	unsigned int addrLen;
	struct udp_message ret;
	struct sockaddr_in addr;
	addrLen = sizeof(addr);
 	memset(&addr, 0, sizeof(addr));
	
	char buf [READSIZE];
	memset(buf, 0, sizeof(buf));
	n = recvfrom(lsocket, buf, READSIZE, 0, (struct sockaddr *) &addr, &addrLen);
	if (n <= 0) {
		error("error receiving from broadcast");
	}
	
	ret.data = (char *) malloc(sizeof(char)*n);
	memcpy(ret.data, buf, sizeof(*ret.data));
	ret.rip= inet_ntoa(addr.sin_addr);
	ret.port = addr.sin_port;
	return ret;	
}

struct udp_message udp_recieve_broadcast(){
	int n;
	unsigned int addrLen;
	struct udp_message ret;
	struct sockaddr_in addr;
 	memset(&addr, 0, sizeof(addr));
	addrLen = sizeof(addr);
	char buf [READSIZE];
	memset(buf, 0, sizeof(buf));
	n = recvfrom(bsocket, buf, READSIZE, 0, (struct sockaddr *) &addr, &addrLen);
	if (n <= 0) {
		error("error receiving from broadcast");
	}

	ret.data = (char *) malloc(sizeof(char)*n);
	memcpy(ret.data, buf, sizeof(*ret.data));
	ret.rip= inet_ntoa(addr.sin_addr);
	ret.port = addr.sin_port;
	return ret;	
}

void udp_close(){
	close(bsocket);
	close(lsocket);

}
