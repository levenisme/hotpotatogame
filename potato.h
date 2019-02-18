#ifndef __POTATO_H__
#define __POTATO_H__
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
using namespace std;
struct hotpotato{
  int hops;
  int end;
  int playerID[512];
};
typedef struct hotpotato hot_potato;




#endif
