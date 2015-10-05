#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void syserr(char *msg) { perror(msg); exit(-1); }

int main(int argc, char *argv[])
{
  int sockfd, clientsockfd, portno, n;
  struct sockaddr_in serv_addr, clt_addr;
  socklen_t addrlen;
  char buffer[256];
  //If port number argument is not given default to 5000
  if(argc != 2) { 
	portno = 5000;
  } 
  else portno = atoi(argv[1]);

  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if(sockfd < 0) syserr("can't open socket"); 
  //printf("create socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    syserr("can't bind");
  //printf("bind socket to port %d...\n", portno);

  listen(sockfd, 5); 

for(;;) {
  //printf("wait on port %d...\n", portno);
  addrlen = sizeof(clt_addr); 
  clientsockfd = accept(sockfd, (struct sockaddr*)&clt_addr, &addrlen);
  if(clientsockfd < 0) syserr("can't accept"); 
  printf("new incoming connection, block on receive\n");
  pid = fork();
  if(pid<0) syserr("fork error");
  else if(pid == 0){
    close(clientsockfd);
  }
  else{
    for(;;){
      n = recv(clientsockfd, buffer, 255, 0); 
      if(n < 0) syserr("can't receive from client"); 
      else buffer[n] = '\0';
      printf("SERVER GOT MESSAGE: %s\n", buffer); 
    }
  }
  //n = send(newsockfd, buffer, sizeof(buffer), 0);
  //if(n < 0) syserr("can't send to server"); 
  //printf("send message...\n");
  close(newsockfd); 
}
  close(sockfd); 
  return 0;
}
