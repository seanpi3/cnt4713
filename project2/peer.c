#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

int serversockfd, serverport;
struct sockaddr_in serv_addr;
void syserr(char* msg) { perror(msg); exit(-1); }

//Logic for the server thread
void *serverLogic(void *arg){
	/*
  int clientsockfd,n;
  struct sockaddr_in clt_addr;
  socklen_t addrlen;
  if(bind(serversockfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
    close(serversockfd);
    syserr("can't bind");
  }
  listen(serversockfd,5);
  for(;;){
    clientsockfd = accept(serversockfd, (struct sockaddr*)&clt_addr,&addrlen);
    addrlen = sizeof(clt_addr);
    if(clientsockfd <0){
	close(clientsockfd);
	printf("Cannot accept client\n");
    }
    else{
	printf("peer connected...\n");
	int pid = fork();
	if(pid<0) syserr("error");
	else if (pid ==0) close(clientsockfd);
	else{
	  char buffer[256];
	  char *filename;
	  char *toke;
	  int filesize;
	  for(;;){
	    memset(buffer,0,sizeof(buffer));
	    n = recv(clientsockfd,buffer,sizeof(buffer),0);
	    if(n<0) {
		printf("Peer lost connection\n");
		close(clientsockfd);
		pthread_exit(NULL);
	    }
	    else buffer[n] = '\0';
	    printf("SERVER GOT MESSAGE: %s\n",buffer);
	  }
	}
    }
  }
  
  
  */
}

int main(int argc, char* argv[])
{
  int trackersockfd,peersockfd,trackerport,peerport, n;
  struct hostent* tracker, peer;
  struct sockaddr_in tracker_addr, peer_addr;
  socklen_t addrlen;
  pthread_t servt;
  char buffer[256];

  if(argc != 4) {
    fprintf(stderr, "Usage: %s <tracker-hostname> <tracker-port> <local-port>\n", argv[0]);
    return 1;
  }
  tracker = gethostbyname(argv[1]);
  if(!tracker) {
    fprintf(stderr, "ERROR: no such host: %s\n", argv[1]);
    return 2;
  }
  trackerport = atoi(argv[2]);
  serverport = atoi(argv[3]);
  /*{
  struct in_addr **addr_list; int i;
  printf("Official name is: %s\n", tracker->h_name);
  printf("    IP addresses: ");
  addr_list = (struct in_addr **)tracker->h_addr_list;
  for(i = 0; addr_list[i] != NULL; i++) {
    printf("%s ", inet_ntoa(*addr_list[i]));
  }
  printf("\n");
  }*/
  
  //Set up tracker and server sockets
  trackersockfd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
  if(trackersockfd < 0) syserr("can't open socket");
  serversockfd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
  if(serversockfd < 0) syserr("can't open socket");
  
  //Set up tracker address
  memset(&tracker_addr, 0, sizeof(tracker_addr));
  tracker_addr.sin_family = AF_INET;
  tracker_addr.sin_addr = *((struct in_addr*)tracker->h_addr);
  tracker_addr.sin_port = htons(trackerport);

  //Set up server address
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(serverport);
  
  //Create thread for the server
  n = pthread_create(&servt,NULL, &serverLogic, NULL);
  if(n<0) syserr("can't create thread");
  
  //Connect to tracker  
  if(connect(trackersockfd, (struct sockaddr*)&tracker_addr, sizeof(tracker_addr)) < 0)
    syserr("can't connect to tracker");
  printf("Connected to tracker.\n");
  for(;;){
  	recv(trackersockfd, buffer,sizeof(buffer),0);
  	printf("CLIENT RECEIVED MESSAE: %s\n", buffer);
  }
  /*
  printf("PLEASE ENTER MESSAGE: ");
  fgets(buffer, 255, stdin);
  n = strlen(buffer); if(n>0 && buffer[n-1] == '\n') buffer[n-1] = '\0';

  n = send(sockfd, buffer, strlen(buffer), 0);
  if(n < 0) syserr("can't send to server");
  printf("send...\n");
  
  n = recv(sockfd, buffer, 255, 0);
  if(n < 0) syserr("can't receive from server");
  else buffer[n] = '\0';
  printf("CLIENT RECEIVED MESSAGE: %s\n", buffer);
  */
  close(trackersockfd);
  close(serversockfd);
  return 0;
}
