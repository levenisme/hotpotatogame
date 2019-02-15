#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "potato.h"
#include "ringmaster.h"
using namespace std;

int main(int argc, char *argv[])
{
  
  const char* port = argv[1];
  int numberP = atoi(argv[2]);
  int numberH = atoi(argv[3]);
  cout<<"numberP: "<<numberP<<endl;
  cout<<"numberH: "<<numberH<<endl;
	
  //initialize potato
  hot_potato potato;
  potato.hops = numberH;
  
  int status;
  int ringmaster_fd;//store the ringmaster's fd
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
 
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
  int YES = 1;
  status = setsockopt(ringmaster_fd, SOL_SOCKET, SO_REUSEADDR, &YES, sizeof(int));
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
  socklen_t socket_addr_len = sizeof(socket_addr);
  int player_fd[numberP];
  player_info players[numberP];//use an array to store all info of players
  //store all info into the struct  
  for(int i=0;i<numberP;i++){
    player_fd[i]=accept(ringmaster_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if(player_fd[i] == -1){
      cerr<<"Error: cannot accept connection on socket" << endl;
      return -1;
    }
    int serPort=3000+i;
    char* buffer=(char*) malloc(100*sizeof(char));
    memset(buffer, 0 ,100);
    snprintf(buffer,100,"%d",serPort);
    char *pport = buffer;
    cout<<"pport:"<<pport<<endl;
    players[i].player_port=pport;
    send(player_fd[i],pport,strlen(pport),0);
    
    char *receiveHost=(char*) malloc(100*sizeof(char));
    memset(receiveHost, 0 ,100);

    //cout<<"receiveHost: "<<receiveHost<<endl;
    
  //receive port number from host                                                                
     recv(player_fd[i], receiveHost, 50, 0);
     //receiveIP[50] = 0;
     players[i].player_host=receiveHost;
     cout<<"receiveHost: "<<players[i].player_host<<endl;
     // cout<< "player number: "<<i<<endl;
  }
  //set the info of the left port to it

  for(int i=0;i<numberP;i++){
    if(i == 0){
      send(player_fd[i],players[numberP-1].player_port,100,0);
      send(player_fd[i],players[numberP-1].player_host,100,0);
    }
    else{
    send(player_fd[i],players[i-1].player_port,100,0);
    send(player_fd[i],players[i-1].player_host,100,0);
    }
   }
  //check the fd[]
  for(int i=0;i<numberP;i++){
   cout<<" player_fd"<<i<<":"<<player_fd[i]<<endl;
   free(players[i].player_port);
   free(players[i].player_host);
  }
  // cout<<"ringmaster_fd: "<<ringmaster_fd<<endl;
  freeaddrinfo(host_info_list);
  close(ringmaster_fd);
  return 0;
}
  
  
