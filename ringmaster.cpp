#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "potato.h"
using namespace std;

int main(int argc, char *argv[])
{
  cout<<"Potato Ringmaster"<<endl;
  int numberP = 0;
  int numberH = 0;
  cout<<"Players = ";
  cin>>numberP;
  cout<<endl;
  cout<<"Hops = ";
  cin>>numberH;
  cout<<endl;

  //initialize potato
  
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
  int client_connection_fd[numberP+10];
  for(int i=0;i<numberP+11;i++){
    client_connection_fd[i]=accept(ringmaster_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if(client_connection_fd[i] == -1){
      cerr<<"Error: cannot accept connection on socket" << endl;
      return -1;
    }
    char buffer[512];
    recv(client_connection_fd[i], buffer, 9, 0);
    buffer[9] = 0;
    cout<< "Server received: "<<buffer<< endl;
    //test
    cout<< "player number: "<<i<<endl;
  }
  freeaddrinfo(host_info_list);
  close(socket_fd);
  return 0;
}
  
  
