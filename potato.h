#ifndef __POTATO_H__
#define __POTATO_H__
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
struct hotpotato{
  int hops;
  int playerID[];
};
typedef struct hotpotato hot_potato;




#endif
