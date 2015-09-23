#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>

void syserr(char *msg) { perror(msg); exit(-1); }



int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, n;
  struct sockaddr_in serv_addr, clt_addr;
  socklen_t addrlen;
  char buffer[256];
  if(argc != 2) { 
    fprintf(stderr,"Usage: %s <port>\n", argv[0]);
    return 1;
  } 
  portno = atoi(argv[1]); //converts portno string to int

  sockfd = socket(AF_INET, SOCK_STREAM, 0); //returns -1 if failed to open socket
  if(sockfd < 0){
	 close(sockfd);
	 syserr("can't open socket");
  }
  printf("Socket created...");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	close(sockfd);
	syserr("can't bind");
  }
  printf("Socket binded to port %d...\n", portno);

  listen(sockfd, 5); 

for(;;){
  newsockfd = accept(sockfd, (struct sockaddr*)&clt_addr, &addrlen);
  addrlen = sizeof(clt_addr); 
  if(newsockfd < 0){
	close(newsockfd);	
 	printf("Cannot accept client");
  } 
 printf("Client connected...\n");
  int pid = fork();
  if(pid<0) syserr("Error");
  else if(pid == 0) {
	close(newsockfd);
  }
  else{	
	char *msg;
	char *command;
    for(;;){
	int found = 0;
  	n = recv(newsockfd, buffer, 255, 0); 
  	if(n < 0) syserr("can't receive from client"); 
  	else buffer[n] = '\0';
	char *toke;  
	printf("SERVER GOT MESSAGE: %s\n", buffer); 
    	toke = strtok(buffer," ,.-");
	command = "stop";
	if(!found && strcmp(toke, command)==0){
		msg = "Server stopping.\n";
		n = send(newsockfd, msg, strlen(msg),0);
		found = 1;
		close(newsockfd);
		close(sockfd);
		printf("server stopping\n");
		exit(0);
	}
	command = "ls";
	msg = "ls-remote";
	if(!found && strcmp(toke,command)==0){
		n = send(newsockfd,msg,strlen(msg),0);
		printf("ls-remote\n");
		found = 1;
	}
	command = "get";
	msg = "get";
	if(!found && strcmp(toke,command)==0){
		n = send(newsockfd,msg,strlen(msg),0);
		printf("getting\n");
		found = 1;
	}
	command = "put";
	msg = "putting";
	if(!found && strcmp(toke,command)==0){
		n = send(newsockfd,msg,strlen(msg),0);
		printf("putting\n");
		found = 1;
	}
	if(!found){
		msg = "Invalid command. Please send one of the following valid commands: ls-remote, get <filename>, put <filename>, stop.";
		n = send(newsockfd, msg, strlen(msg),0);
	}
    }
  	close(newsockfd); 
  }
}
  close(sockfd); 
  return 0;
}
