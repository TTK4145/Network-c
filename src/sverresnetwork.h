#ifndef SVERRESNETWORK_H
#define SVERRESNETWORK_H

// Note that the callback functions you provide may be called by the
// threads created by this module. Synchronize the access to any
// resources.

typedef void (*TMessageCallback)(const char * ip, char * data, int datalength);
typedef void (*TTcpConnectionCallback)(const char * ip, int created);


// Misc
char * getMyIpAddress(char * interface); // NB: allocates the return string!

// UDP & Broadcast
void udp_startReceiving(int port,TMessageCallback callBack);
void udp_send(char * address,int port,char * data, int dataLength);
void udp_broadcast(int port,char * data, int dataLength);


// TCP 
void tcp_init(TMessageCallback messageCallback, TTcpConnectionCallback connectionCallback);
void tcp_startConnectionListening(int port);
void tcp_openConnection(char * ip,int port);
void tcp_send(char * ip,char * data, int datalength);


#endif
