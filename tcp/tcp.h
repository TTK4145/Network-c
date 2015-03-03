#ifndef _UDP__H
#define _UDP__H

#define READSIZE 1024

struct tcp_message{
	char *rip;
	int port;
	char * data;
	int length;


};



void tcp_init();
void tcp_close();

//temp availability
//int ip_to_last_subnet_int (char * ip);


#endif /* _UDP__H */
