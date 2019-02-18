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
  if(numberP<=1||numberH<0||numberH>512){
    cerr<<"invalid input"<<endl;
    return -1;
  }
  cout<<"Potato Ringmaster"<<endl;
  cout<<"Players = "<<numberP<<endl;
  cout<<"Hops = "<<numberH<<endl;
      
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
  status =  listen(ringmaster_fd, 200);
  if(status == -1){
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }
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
    send(player_fd[i],&numberP,sizeof(int),0);
    cout<<"Player "<<i<<" is ready to play"<<endl;
    int serPort=3000+i;
    char* buffer=(char*) malloc(100*sizeof(char));
    memset(buffer, 0 ,100);
    snprintf(buffer,100,"%d",serPort);
    char *pport = buffer;

    players[i].player_port=pport;
    send(player_fd[i],pport,strlen(pport),0);
    
    char *receiveHost=(char*) malloc(100*sizeof(char));
    memset(receiveHost, 0 ,100);
    recv(player_fd[i], receiveHost, 50, 0);
    players[i].player_host=receiveHost;
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
  //send the potato to a random player
  srand((unsigned int)time(NULL));
  int random = rand() % numberP;
  hot_potato potato;
  memset(&potato, 0, sizeof(hot_potato));
  if(numberH==0){
    potato.end = 1;
  }
  else{
    potato.hops = numberH-1;
    potato.end = 0;
    cout<<"Ready to start the game, sending potato to player "<<random<<endl;
    send(player_fd[random],&potato,sizeof(hot_potato),0);
    //receive the potato from the last player
    fd_set fds;
    FD_ZERO(&fds);
    for(int i=0;i<numberP;i++){
    FD_SET(player_fd[i],&fds);
    }
    if(numberH==0){
      potato.end = 1;
    }
    int check = select(sizeof(fds)*(numberP+1), &fds, NULL, NULL, NULL);
    if(check == -1){
      cerr<<"fail select in ringmaster"<<endl;
      return -1;
    }
    int it_fd;
    for(int i=0;i<numberP;i++){
      if(FD_ISSET(player_fd[i],&fds)) it_fd=player_fd[i];
    }
    if(recv(it_fd,&potato, sizeof(hot_potato),0)<0){
      cerr<<"cannot receive potato"<<endl;
    }
    potato.end = 1;
    cout<<"Trace of potato:"<<endl;
    cout<<potato.playerID[numberH-1];
    for(int i = numberH-2;i >= 0;i--){
      cout<<","<<potato.playerID[i];
    }
    cout<<endl;
  }
 
  for(int i=0;i<numberP;i++){
   send(player_fd[i],&potato,sizeof(hot_potato),0);
  }

  
  for(int i=0;i<numberP;i++){
   free(players[i].player_port);
   free(players[i].player_host);
  }
  // cout<<"ringmaster_fd: "<<ringmaster_fd<<endl;
  //  free(potato.playerID);
  freeaddrinfo(host_info_list);
  close(ringmaster_fd);
  return 0;
}
  
  
