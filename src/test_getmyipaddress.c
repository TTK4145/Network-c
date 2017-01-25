#include <stdio.h>
#include <stdlib.h>
#include "sverresnetwork.h"

int main(){
  char * ip = getMyIpAddress("enp4s0");
  printf("My Ip is %s\n",ip);

  free(ip);

  return 0;
}

