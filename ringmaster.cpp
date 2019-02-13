#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "potato.h"
#include <stdio.h>
int main(int argc, char *argv[])
{
  int status;
  int ringmaster_fd;//store the ringmaster's fd
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port = "4444";//need to find whether should use this port
  memset(&host_info,0,sizeof(host_info));
  
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if(status != 0){
    cerr<<"Error: cannot get address info for ringmaster"<<endl;
    return -1;
  }

  ringmaster_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,host_info_list->ai_protocol);
  //should pass it to clients
  if(ringmaster_fd==-1){
    cerr<<"Error: cannot create ringmaster's socket"<<endl;
    return -1;
  }

  status = setsockeopt(ringmaster_fd, SOL_SOCKET, SO_REUSEADDR, &YES, sizeof(int));
  status = bind(ringmaster_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if(status == -1){
    cerr<<"Error: cannot bind socket of ringmaster"<<endl;
    return -1;
  }

  // printf("waiting for players\n");

  status =  listen(ringmaster_fd, 200);
  if(status == -1){
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }

  cout<<"waiting for players"<<endl;
  
  struct sockaddr_storage socket_addr;
  socklen_t socket_add_len = sizeof(sock_addr);
  int client_connection_fd[]
