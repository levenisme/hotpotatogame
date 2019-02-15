#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "potato.h"
#include <stdlib.h>
#include "ringmaster.h"
using namespace std;
int beServer(char* port,int ringmaster_fd){
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  // char buffer [33];
  //snprintf(buffer,sizeof(buffer),"%d",p);
  // char *port = buffer;
  cout<<"port: "<<port<<endl;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;
  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
   return -1;
  } //if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }
  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  
  char myhost[100];
  gethostname(myhost, 100);
  send(ringmaster_fd,myhost,strlen(myhost),0);
  cout<<"host:"<<myhost<<endl;
  cout << "become a server: " << port << endl;
  /*  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return;
  } //if
  */
  freeaddrinfo(host_info_list);
  return socket_fd;
}
int beClientofL(char* playerL_host,char* playerL_port){
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = playerL_host;
  const char *port = playerL_port;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }
  
  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    freeaddrinfo(host_info_list);
    return -1;
  }
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    freeaddrinfo(host_info_list);
    return -1;
  }
  freeaddrinfo(host_info_list);
  cout<<"connect to the left"<<endl;
  return socket_fd;
}

void sendMessage(const char * message, int socked_fd){
  send(socked_fd, message, 100,0);
}
void receiveMessage(int fd){
  char buffer[100] = {0};
  recv(fd, buffer, 100, 0);
  cout << "Server received: " << buffer << endl;
}
int main(int argc, char *argv[])
{
  int status;
  int ringmaster_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = argv[1];
  const char *port = argv[2];

  if(argc < 2){
    cout << "Syntax: client <hostname>\n" << endl;
      return 1;
  }
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if(status != 0){
    cerr << "Error: cannot get address info for server" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }

  ringmaster_fd = socket(host_info_list->ai_family,host_info_list->ai_socktype,host_info_list->ai_protocol);
  if(ringmaster_fd == -1){
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //
  cout << "conneting to "<<hostname<<"on port"<<port<<"..."<<endl;
  status = connect(ringmaster_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }
  //  const char *message = "hi there!";
  //send(ringmaster_fd,message,strlen(message),0);
  char receivefd[512];

  //receive port number from host, use this number to build a socket
  recv(ringmaster_fd, receivefd, 10, 0);
  receivefd[10] = 0;
  cout<<"receivefd: "<<receivefd<<endl;
  char* myport = receivefd;
  
  cout << "Waiting for connection on port " << myport << endl;
  int server_fd = beServer(myport,ringmaster_fd);
  cout << "I am a server now"<<endl;
  cout << "my port is: "<<myport<<endl;
  //accept connection
  /*  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd = accept(server_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return -1;
  } //if
  cout << "I connect now!"<<endl;
  */
  char playerL_port[100];
  recv(ringmaster_fd, playerL_port, 100, 0);
  char playerL_host[100];
  recv(ringmaster_fd, playerL_host, 100, 0);
  int myplayerL_fd = beClientofL(playerL_host,playerL_port);
  //beServer(myport,ringmaster_fd);
  cout<<"myplayerL_port: "<<playerL_port<<endl;
  cout<<"myplayerL_host: "<<playerL_host<<endl;
  //accept connection                                                                                              
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int myplayerR_fd;
  myplayerR_fd = accept(server_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (myplayerR_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return -1;
  } //if                                                                                                           
  cout << "I connect now!"<<endl;
 const char * messager = "I am your left ";
  sendMessage(messager, myplayerR_fd);
  const char * messagel = "I am your right: ";
  sendMessage(messagel, myplayerL_fd);

  receiveMessage(myplayerR_fd);
   receiveMessage(myplayerL_fd);
  
  freeaddrinfo(host_info_list);
  close(ringmaster_fd);

  return 0;
 }
  
