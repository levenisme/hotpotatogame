#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "potato.h"
#include <stdlib.h>
#include "ringmaster.h"
using namespace std;
//use the port that the ringmaster assigned to build a socket and listen
int beServer(char* port,int ringmaster_fd){
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;
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
   return -1;
  } 

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
  } 
  //send the infomation of host to ringmaster
  char myhost[100];
  gethostname(myhost, 100);
  send(ringmaster_fd,myhost,strlen(myhost),MSG_WAITALL);
  freeaddrinfo(host_info_list);
  return socket_fd;
}
//connect the left neighbour
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
  return socket_fd;
}

int main(int argc, char *argv[])
{
  //connect ringmaster
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
  status = connect(ringmaster_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }
  int numberP=0;
  //receive the information of the number of players 
  recv(ringmaster_fd,&numberP,sizeof(int),0);

  char receivefd[512];
  recv(ringmaster_fd, receivefd, 10, 0);
  receivefd[10] = 0;
  char* myport = receivefd;
  int mynumber = atoi(myport)-3000;
  cout<<"Connected as player "<<mynumber<<" out of "<<numberP<<" total players"<<endl;
  //differentiate the neighbours
  int myleft = mynumber == 0? numberP-1:mynumber-1;
  int myright = mynumber == numberP-1? 0:mynumber+1;
  int server_fd = beServer(myport,ringmaster_fd);
  char playerL_port[100];
  recv(ringmaster_fd, playerL_port, 100, 0);
  char playerL_host[100];
  recv(ringmaster_fd, playerL_host, 100, 0);
  int myplayerL_fd = beClientofL(playerL_host,playerL_port);

  //accept connection from the right neighbor
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int myplayerR_fd;
  myplayerR_fd = accept(server_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (myplayerR_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return -1;
  }
  //store neighbor's fd
  fd_set fds;
  int n_fds[2];
  n_fds[0] = myplayerL_fd;
  n_fds[1] = myplayerR_fd;
  hot_potato potato;
  memset(&potato, 0, sizeof(hot_potato));
  srand((unsigned int)time (NULL));
  while(1){
    //receive potato either from the ringmaster or the neighbors
    FD_ZERO(&fds);
    FD_SET(ringmaster_fd,&fds);
    FD_SET(myplayerR_fd,&fds);
    FD_SET(myplayerL_fd,&fds);
    int check = select(sizeof(fds)*4, &fds, NULL, NULL, NULL);
    if(check == -1){
      cerr<<"fail select"<<endl;
      return -1;
    }
    int receive_fd;
    if(FD_ISSET(ringmaster_fd,&fds)) receive_fd = ringmaster_fd;
    else if(FD_ISSET(myplayerL_fd,&fds)) receive_fd = myplayerL_fd;
    else receive_fd = myplayerR_fd;
    if(recv(receive_fd,&potato, sizeof(hot_potato),0)<0){
      cerr<<"cannot receive potato"<<endl;
    }
    //game end, need to close
    if(potato.end == 1){
      break;
    }
    //receive the potato, send the potato to neighbors
    else if(potato.hops>0){
      int random = rand()%2;
      int mynumber = atoi(myport)-3000;
      potato.playerID[potato.hops--] = mynumber;
      int nextnumber = (random == 0)? myleft:myright;
      cout<<"Sending potato to "<<nextnumber<<endl;
      send(n_fds[random],&potato,sizeof(hot_potato),MSG_WAITALL);
    }
    //game end, return the potato to the ringmaster
    else if(potato.hops == 0){
      int mynumber = atoi(myport)-3000;
      potato.playerID[0] = mynumber;
      cout<<"I'm it"<<endl;
      send(ringmaster_fd,&potato,sizeof(hot_potato),MSG_WAITALL);
    }
  }
  freeaddrinfo(host_info_list);
  close(ringmaster_fd);
  
  return 0;
 }
  
