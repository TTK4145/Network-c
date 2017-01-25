#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <assert.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "sverresnetwork.h"

#include "sverresnetwork.h"

void 
error(char *s)
{
  perror(s);
  exit(1);
}

int m_log = 1;

void 
setLogMode(int l){
  m_log = l;
}
char * 
getMyIpAddress(char * interface)
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;

    assert(strlen(interface) < 20);

    getifaddrs (&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            if(strncmp(ifa->ifa_name,interface,strlen(ifa->ifa_name)) == 0){
              char * res = strdup(addr);
              freeifaddrs(ifap);
              return res;
            }
//            printf("Interface: %s\tAddress: '%s' - '%s'\n", ifa->ifa_name, addr, interface);
        }
    }
    fprintf(stderr,"\n\nERROR: getMyIpAddress: Could not find interface %s.\nAlternatives are:\n",interface);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            fprintf(stderr,"Error: Interface: %8s\tAddress: %s\n", ifa->ifa_name, addr);
        }
    }
    fprintf(stderr,"\n");

    freeifaddrs(ifap);
    assert(0);
    return 0;
}


// Need to keep track of running listening threads.
typedef struct {
  int port;
  int socket;
  TMessageCallback callBack;
  pthread_t thread;
} TUdpThreadListItem;

#define NLISTENINGTHREADS 100
#define BUFLEN 512

TUdpThreadListItem * listeningThreads[NLISTENINGTHREADS];
int nOfListeningThreads = 0;

void * 
thr_udpListen(void * arg){

  TUdpThreadListItem * threadListItem = (TUdpThreadListItem*) arg;

  struct sockaddr_in si_me, si_other;
  socklen_t slen = sizeof(si_other);
  char buf[BUFLEN];

  threadListItem->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  assert(threadListItem->socket != -1);

  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(threadListItem->port);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);

  int optval = 1;
  setsockopt(threadListItem->socket,SOL_SOCKET,SO_REUSEADDR, &optval, sizeof(optval));

  int res = bind(threadListItem->socket,(struct sockaddr *) &si_me, sizeof(si_me));
  if(res == -1) error("thr_udpListen:bind");

  if(m_log) printf("Sverresnetwork: udpListen: Binding to port %d\n",threadListItem->port);

  while(1){
    res = recvfrom(threadListItem->socket, buf, BUFLEN, 0,(struct sockaddr *) &si_other, &slen);
    if(res == -1) error("thr_udpListen:recvfrom");
    if(res >= BUFLEN-1){
      fprintf(stderr,"recvfrom: Hmm, length of received message is larger than max message length: %d vs %d\n\n",res,BUFLEN);      
      assert(res < BUFLEN-1);
    }
    // printf("Received packet from %s:%d\nLength = %d, Data: <%s>\n\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), res,buf);
     (*(threadListItem->callBack))(inet_ntoa(si_other.sin_addr),buf,res);
  }

  // Never executed - this thread will be killed if it is not needed any more.
  close(threadListItem->socket);
  return 0;
}

void udp_startReceiving(int port,TMessageCallback callBack){
  
  assert(nOfListeningThreads<NLISTENINGTHREADS);

  listeningThreads[nOfListeningThreads] = (TUdpThreadListItem*) malloc(sizeof(TUdpThreadListItem));
  assert(listeningThreads[nOfListeningThreads] != NULL);

  listeningThreads[nOfListeningThreads]->port = port;
  listeningThreads[nOfListeningThreads]->callBack = callBack;

  int res = pthread_create(&(listeningThreads[nOfListeningThreads]->thread), NULL, thr_udpListen, listeningThreads[nOfListeningThreads]);
  if(res != 0) error("pthread_create failed");

  nOfListeningThreads++;
}


void 
udp_send(char * address,int port,char * data, int dataLength){
  struct sockaddr_in si_other;
  int s, slen=sizeof(si_other);

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    error("udp_send: socket");

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(port);
  int res = inet_aton(address, &si_other.sin_addr);
  if(res==0) error("inet_aton() failed\n");

  res = sendto(s, data, dataLength, 0, (struct sockaddr *) &si_other, slen);
  if(res==-1)  error("udp_send: sendto()");

  close(s);
}


void 
udp_broadcast(int port,char * data, int dataLength){
  struct sockaddr_in si_other;
  int s, slen=sizeof(si_other);

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    error("udp_send: socket");

  int optval = 1;
  setsockopt(s,SOL_SOCKET,SO_BROADCAST, &optval, sizeof(optval));

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(port);
  if (inet_aton("255.255.255.255", &si_other.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }

  int res = sendto(s, data, dataLength, 0, (struct sockaddr *) &si_other, slen);
  if(res==-1)  error("udp_broadcast: sendto()");

  close(s);
}

static TMessageCallback m_messageCallback = NULL;
static TTcpConnectionCallback m_connectionCallback = NULL;

#define MAXTCPCONNECTIONS 100

typedef struct {
  char ip[32];
  int socket;
} TTcpConnection;

static TTcpConnection tcpConnections[MAXTCPCONNECTIONS];

void
conn_init(){
  int i;
  for(i=0;i<MAXTCPCONNECTIONS;i++) tcpConnections[i].socket=0;
}

int 
conn_lookup(char * ip){
  int i;
  for(i=0;i<MAXTCPCONNECTIONS;i++){
    if(strncmp(tcpConnections[i].ip,ip,30) == 0 && tcpConnections[i].socket != 0){
      return tcpConnections[i].socket;
    }
  }
  return 0;
}

const char *
conn_findIp(int socket){
  int i;
  for(i=0;i<MAXTCPCONNECTIONS;i++){
    if(tcpConnections[i].socket == socket){
      return tcpConnections[i].ip;
    }
  }
  return NULL;
}


void
conn_add(char * ip,int s){
  int i;

  if(conn_lookup(ip))
    error("conn_add: Trying to add an already existing connection");

  for(i=0;i<MAXTCPCONNECTIONS;i++){
    if(tcpConnections[i].socket == 0){
      strncpy(tcpConnections[i].ip,ip,30);
      tcpConnections[i].socket = s;
      return;
    }
  }
  error("conn_add: No free connection slots?");
}

void
conn_remove(const char * ip){
  int i;
  for(i=0;i<MAXTCPCONNECTIONS;i++){
    if(strncmp(tcpConnections[i].ip,ip,30) == 0){
      tcpConnections[i].socket = 0;
    }
  }
}


void * 
thr_tcpMessageListen(void * parameter){
  int socket = (long) parameter;
  char buffer[1024];

  if(m_log) printf("SverresNetwork: Starting reading messages from socket %d\n",socket);
  
  while(1){
    bzero(buffer,1024);
    int n = read(socket,buffer,1020);
    if(n <= 0){
      // Something wrong; close and give up
      close(socket);
      if(m_log) printf("SverresNetwork: Lost a connection to %s - socket %d\n",conn_findIp(socket),socket);
      m_connectionCallback(conn_findIp(socket),0);
      conn_remove(conn_findIp(socket));
      return NULL;
    } 
  
    // Got a message, do the callback
    m_messageCallback(conn_findIp(socket),buffer,n);
  }
}

void * 
thr_tcpConnectionListen(void * parameter){
  int sockfd, newsockfd, port;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  pthread_t thread;

  port = (long) parameter;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) 
     error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  int optval = 1;
  setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &optval, sizeof(optval));

  if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  listen(sockfd,5);

  if(m_log) printf("SverresNetwork: Starting listening for connections on %d\n",port);

  clilen = sizeof(cli_addr);
  while(1){
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if(newsockfd < 0) 
      error("ERROR on accept");

    // Now: Have the connection! 
    char * newConnectionIp = strdup(inet_ntoa(cli_addr.sin_addr));
    if(m_log) printf("SverresNetwork: Got a connection from %s: socket %d\n",newConnectionIp,newsockfd);

    conn_add(newConnectionIp,newsockfd);

    // Notify the system
    if(m_log) printf("SverresNetwork: Created a connection to %s - socket %d\n",newConnectionIp,newsockfd);
    m_connectionCallback(newConnectionIp,1);

    // And create the thread that will receive the messages.
    long lfd = newsockfd;
    int res = pthread_create(&thread, NULL, thr_tcpMessageListen,(void *) lfd);
    if(res != 0) error("pthread_create failed");
  }

}


void 
tcp_init(TMessageCallback messageCallback, TTcpConnectionCallback connectionCallback){
  m_messageCallback = messageCallback;
  m_connectionCallback = connectionCallback;  
  conn_init();
}


void 
tcp_startConnectionListening(int port){
  pthread_t thread;

  long lfd = port;
  int res = pthread_create(&thread, NULL, thr_tcpConnectionListen,(void *) lfd);
  if(res != 0) error("pthread_create failed");
}


void 
tcp_openConnection(char * ip,int port){
  int sockfd;
  struct sockaddr_in serv_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) 
    error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  int res = inet_aton(ip, &serv_addr.sin_addr);
  if(res==0) error("inet_aton() failed\n");
  serv_addr.sin_port = htons(port);

  res = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
  if(res < 0){
    // error("ERROR connecting");
    if(m_log) fprintf(stderr,"WARNING: sverresnetwork: Tried to open connection to %s:%d but failed\n",ip,port);
    return;
  }

  // Have a connection; register it and make the callback.
  conn_add(ip,sockfd);

  // Notify the system
  if(m_log) printf("SverresNetwork: Created a connection to %s - socket %d\n",ip,sockfd);
  m_connectionCallback(ip,1);
  
  // And create the thread that will receive the messages.
  long lfd = sockfd;
  pthread_t thread;
  res = pthread_create(&thread, NULL, thr_tcpMessageListen,(void *) lfd);
  if(res != 0) error("pthread_create failed");

}


void 
tcp_send(char * ip,char * data, int datalength){
  int socket = conn_lookup(ip);

  if(socket==0){
    // The connection is nonexistent: Must notify
    if(m_log) printf("SverresNetwork: Tried to write to a nonexistant connection: %s\n",ip);
    m_connectionCallback(ip,0);
  }

  int res = write(socket,data,datalength);
  if(res != datalength){
    if(m_log) printf("SverresNetwork: Writing failed to %s:%d Got %d/%d Closing\n",ip,socket,res,datalength);
//    error("write:");
    if(m_log) printf("SverresNetwork: Closed a connection to %s - socket %d\n",ip,socket);
    m_connectionCallback(ip,0);
    conn_remove(ip);
  }
}


