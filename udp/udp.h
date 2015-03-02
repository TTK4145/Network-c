#ifndef _UDP__H
#define _UDP__H

#define READSIZE 1024

struct udp_message{
	char *rip;
	int port;
	char * data;
	int length;


};



void udp_init();
int udp_sendTo(char * ip, int port, char * msg, int length);
struct udp_message udp_receive_local();
struct udp_message udp_recieve_broadcast();
void udp_close();





#endif /* _UDP__H */
