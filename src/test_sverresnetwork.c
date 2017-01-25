#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "sverresnetwork.h"

char * myIp;

void 
messageReceived(const char * ip, char * data, int datalength){
  printf("Received message from %s: '%s'\n",ip,data);
  if(strncmp("Hello",data,5) == 0 && strncmp(myIp,ip,strlen(myIp)) != 0 ){
    printf("Received Hello message: Connecting...\n");
    tcp_openConnection((char *)ip,5555); // Cast away the const...
  }
}

char ips[100][32];

void 
initIps(){
  int i;
  for(i=0;i<100;i++){
    ips[i][0] = 0;
  }
}

void 
addIp(const char * ip){
  int i;
  for(i=0;i<100;i++){
    if(ips[i][0] == 0){
      printf("Adding connection %s to index %d\n",ip,i);
      strncpy(ips[i],ip,30);
      return;
    }
  }
  assert(0);
}
void 
removeIp(const char * ip){
  int i;
  for(i=0;i<100;i++){
    if(strncmp(ip,ips[i],strlen(ips[i])) == 0 && ips[i][0] != 0){
      printf("Removing connection %s from index %d\n",ip,i);
      ips[i][0] = 0;
      return;
    }
  }
}


void 
connectionStatus(const char * ip, int status){
  printf("A connection got updated %s: %d\n",ip,status);
  if(status){
    addIp(ip);
  }else{
    removeIp(ip);
  }
}


int main(){
  int i;
  initIps();

  int myTcpPortNumber = 5555;

  myIp = getMyIpAddress("enp4s0");
//  myIp = getMyIpAddress("eth0");
  printf("My Ip is %s\n",myIp);

  udp_startReceiving(5555,messageReceived);

  char buffer[1024];

  sprintf(buffer,"Hello: Contact me at %s:%d",myIp,myTcpPortNumber);
  printf("Broadcasting my address: '%s'\n",buffer);
  udp_broadcast(5555,buffer,strlen(buffer)+1);

  tcp_init(messageReceived,connectionStatus);
  tcp_startConnectionListening(5555);

  printf("Going into send loop \n");

  while(1){
    sleep(4);
    printf("Slept: Going into send loop \n");
    for(i=0;i<100;i++){
      if(ips[i][0] != 0){
        printf("Sending a message to %s\n",ips[i]);
        tcp_send(ips[i],"Message",7);
      }
    }
  }

  sleep(100);
  return 0;
}

