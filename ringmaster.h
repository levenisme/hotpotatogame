#ifndef __RINGMASTER_H__
#define __RINGMASTER_H__
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
struct player{
  char* player_host;
  char* player_port;
};
typedef struct player player_info;




#endif
