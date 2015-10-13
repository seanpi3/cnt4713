#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int sockfd;
void syserr(char *msg) { perror(msg); exit(-1); }
void *trackerLogic(void *arg){
  int clientsockfd, n;
  socklen_t addrlen;
  char buffer[256];
  clientsockfd = sockfd;
  printf("client connected\n");
}

int main(int argc, char *argv[])
{
  int clientsockfd, portno, n;
  struct sockaddr_in serv_addr, clt_addr;
  socklen_t addrlen;
  pthread_t clt_thread;
  char buffer[256];

  //If port number argument is not given default to 5000
  if(argc < 2) portno = 5000; 
  else if(argc == 2) portno = atoi(argv[1]);
  else fprintf(stderr, "Usage: %s <port>\n",argv[0]);

  //Create server socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if(sockfd < 0) syserr("can't open socket"); 
  //printf("create socket...\n");

  //Set up server address
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  //Bind socket to port
  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    syserr("can't bind");
  //printf("bind socket to port %d...\n", portno);

  listen(sockfd, 5); 
  printf("Tracker set up and waiting for clients...\n");
//Accept and set up incoming clients
for(;;) {
  //printf("wait on port %d...\n", portno);
  clientsockfd = accept(sockfd, (struct sockaddr*)&clt_addr, &addrlen);
  addrlen = sizeof(clt_addr); 
  if(clientsockfd < 0){
    close(clientsockfd);
    printf("Cannot accept client\n");
  } 
  else{
    printf("New incoming connection...");
    
    //Create thread for incoming client
    n = pthread_create(&clt_thread,NULL,&trackerLogic, NULL);
    if(n<0) syserr("can't create thread");
  }
}
  close(sockfd); 
  return 0;
}