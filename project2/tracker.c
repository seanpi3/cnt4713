#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int sockfd;
typedef struct fileList{
  char *IP;
  char *port;
  char *filename;
  struct fileList *nextFile;
};
typedef struct client{
	int sockfd;
	struct sockaddr_in clt_addr;
	socklen_t addrlen;
};
struct fileList *head;
struct fileList *tail;
void syserr(char *msg) { perror(msg); exit(-1); }
void *trackerLogic(void *arg){
  int n;
  struct client *clt;
  clt = malloc(sizeof(clt));
  clt = (struct client *)arg;
  char buffer[256];
  char *clientIP;
  memset(buffer,0,256);
  printf("client connected\n");
  clt->addrlen = sizeof(clt->clt_addr);
  n = getpeername(clt->sockfd,(struct sockaddr *)&(clt->clt_addr),&(clt->addrlen));
  //clientIP = malloc(20*sizeof(char));
  // recv(clt.sockfd, buffer, sizeof(buffer),0);
  //strcpy(clientIP, inet_ntoa(clt->clt_addr.sin_addr));
  printf("%d",clt->sockfd);
}
int main(int argc, char *argv[])
{
  int clientsockfd, portno, n;
  struct client *clt;
  struct sockaddr_in serv_addr;
  pthread_t clt_thread;
  char buffer[256];
  head = malloc(sizeof(struct fileList));
  head = tail;
  //If port number argument is not given default to 5000
  if(argc < 2) portno = 5000; 
  else if(argc == 2) portno = atoi(argv[1]);
  else fprintf(stderr, "Usage: %s <port>\n",argv[0]);

  //Create server socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if(sockfd < 0) syserr("can't open socket"); 

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
  clt = malloc(sizeof(clt));
for(;;) {
  clt->sockfd = accept(sockfd, (struct sockaddr*)&(clt->clt_addr), &(clt->addrlen));
  //printf("wait on port %d...\n", portno);
  clt->addrlen = sizeof(clt->clt_addr);
  if(clt->sockfd < 0){
    close(clt->sockfd);
    printf("Cannot accept client\n");
  } 
  else{
    printf("New incoming connection...");
    //Create thread for incoming client
    n = pthread_create(&clt_thread,NULL,&trackerLogic,(void *)clt);
    if(n<0) syserr("can't create thread");
  }
}
  close(sockfd); 
  return 0;
}
