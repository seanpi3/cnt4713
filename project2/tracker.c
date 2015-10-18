#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

typedef struct client {
	int sockfd, addr;
	struct sockaddr_in clt_addr;
	socklen_t addrlen;
};
typedef struct fileList{
	char *IP, *port, *filename;
	struct fileList *nextFile;
};
//logic for each connected client, run on a new thread
void *trackerLogic(void *arg){
  char *clientAddress;
  char buffer[256];
  printf("Client connected...\n");
  struct client *clt = (struct client *)arg;
  clientAddress = malloc(sizeof(char) * 20);
  strcpy(clientAddress, inet_ntoa(clt->clt_addr.sin_addr));
  printf("%s\n",inet_ntoa(clt->clt_addr.sin_addr));
  printf("%s connected",clientAddress);
  //send(clt->sockfd,buffer,sizeof(buffer),0);
}

void syserr(char *msg) { perror(msg); exit(-1); }
int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, n;
  struct sockaddr_in serv_addr, clt_addr;
  socklen_t addrlen;
  pthread_t clt_thread;
  //Default to port 5000 if no port is given
  if(argc == 1) portno = 5000;
  else if(argc > 2) { 
    fprintf(stderr,"Usage: %s <port>\n", argv[0]);
    return 1;
  } 
  else portno = atoi(argv[1]); //converts portno string to int

  //Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0); //returns -1 if failed to open socket
  if(sockfd < 0){
	 close(sockfd);
	 syserr("can't open socket");
  }

  //Set up server address
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  //Bind socket to port
  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	close(sockfd);
	syserr("can't bind");
  }

  //Listen for incoming clients
  listen(sockfd, 5); 
  printf("Tracker set up and waiting for clients...\n");
  struct client *clt;
  clt = malloc(sizeof(struct client));
for(;;){
  clt->sockfd = accept(sockfd, (struct sockaddr*)&(clt->clt_addr), &(clt->addrlen));
  clt->addrlen = sizeof(clt->clt_addr); 
  if(clt->sockfd < 0){
	close(clt->sockfd);	
 	printf("Cannot accept client\n");
  }
 else{
	 printf("New incoming connection...");
	 n = pthread_create(&clt_thread,NULL,&trackerLogic,clt);
  }
}
  close(sockfd); 
  return 0;
}
