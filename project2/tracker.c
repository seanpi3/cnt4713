#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
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
	int sockfd;
	struct sockaddr_in clt_addr;
	socklen_t addrlen;
 	 pthread_t clt_thread;
};
typedef struct fileList{
	char *IP, *filename;
	int port;
	struct fileList *nextFile;
};
struct fileList *head;
struct fileList *tail;
void lostconn(char *ip){
	printf("%s lost connection\n",ip);
	struct fileList *current;
	struct fileList *last;
	current = head;
	last = NULL;
	while(current!=NULL){
		if(strcmp(current->IP,ip)==0){
			if(last==NULL){
				head=current->nextFile;
				free(current);
				current=head;
			}
			else{
				last->nextFile=current->nextFile;
				free(current);
				current = last->nextFile;
			}
		}
		else{
			current = current->nextFile;
		}
	}
	pthread_exit();
}
void printlist(){
	struct fileList *current = head;
	while(current != NULL){
		printf("%s at client %s\n",current->filename,current->IP);
		current = current->nextFile;
	}
}
//logic for each connected client, run on a new thread
void *trackerLogic(void *arg){
  char buffer[256];
  char *msg;
  char *addr;
  int n;
  uint32_t countIn, fileCount;
  struct client *clt = (struct client *)arg;
  n = getpeername(clt->sockfd,(struct sockaddr_in *)&(clt->clt_addr),&(clt->addrlen));
  addr = inet_ntoa(clt->clt_addr.sin_addr);
  printf("%s connected\n",addr);
  msg = malloc(sizeof(buffer));


	//Request port from client
	memset(msg,0,sizeof(buffer));
  msg = "reqport\0";
  n = send(clt->sockfd,msg,sizeof(buffer),0);
  if(n<0) lostconn(addr);
	//printf("Sent %d bytes to client\n", n);
	uint32_t portIn;
	n = recv(clt->sockfd,&portIn,sizeof(uint32_t),0);
  if(n<0||n==0) lostconn(addr);
	//printf("Received %d bytes from client\n",n);
 	uint32_t port;
  port = ntohl(portIn);
//  printf("Port: %d\n",port);

	//Request files from client
  //memset(msg,0,sizeof(buffer));
  msg = "reqfiles\0";
  n = send(clt->sockfd,msg,sizeof(buffer),0);
  if(n<0) lostconn(addr);
  n = recv(clt->sockfd,&countIn,sizeof(uint32_t),0);
  if(n<0||n==0)lostconn(addr);
  fileCount = ntohl(countIn);
	//printf("Expecting %d files\n",fileCount);
  while(fileCount > 0){
    memset(buffer,0,sizeof(buffer));
    n = recv(clt->sockfd,buffer,sizeof(buffer),0);
		if(n<0||n==0) lostconn(addr);
		//printf("Received %d bytes from client...",n);
		if(head==NULL){
			head = malloc(sizeof(struct fileList));
			head->filename = malloc(sizeof(buffer));
			strcpy(head->filename,buffer);
			head->IP = addr;
			head->port = port;
			head->nextFile = NULL;
			tail = head;
			//printf("Added %s to list",head->filename);
		}
		else{
			tail->nextFile = malloc(sizeof(struct fileList));
			tail = tail->nextFile;
			tail->filename = malloc(sizeof(buffer));
			strcpy(tail->filename, buffer);
			tail->IP = addr;
			tail->port = port;
			tail->nextFile = NULL;
			//printf("Added %s to list",tail->filename);
		}
		fileCount --;
  }
	//printf("Files added to list.\n");
  //printf("Received all files\n",buffer); 
	//printfiles();
	//printf("everything set up waiting for commands and other clients...\n");
	for(;;){
		memset(buffer,0,sizeof(buffer));
		n = recv(clt->sockfd,buffer,sizeof(buffer),0);
		if(n<=0) lostconn(addr);
		//command conditionals
		if(strcmp(buffer,"reqlist")==0){
			printf("Sending list to %s\n",addr);
			struct fileList *current = head;
			while(current != NULL){
				n = send(clt->sockfd,current->filename,sizeof(buffer),0);
				if(n<0) lostconn(addr);
				//printf("sent: %s\n",current->filename);
				//printf("sent %d bytes to client\n",n);
				n = send(clt->sockfd, current->IP,sizeof(buffer),0);
				if(n<0) lostconn(addr);
				//printf("sent: %s\n", current->IP);
				//printf("sent %d bytes to client\n",n);
				uint32_t portOut = htonl((uint32_t)current->port);
				n = send(clt->sockfd, &portOut, sizeof(uint32_t),0);
				if(n<0) lostconn(addr);
				//printf("send: %d\n",current->port);
				//printf("sent %d bytes to client\n",n);
				current = current->nextFile;
				if(current==NULL) {
					msg = "EOL\0";
					n = send(clt->sockfd, msg, sizeof(buffer),0);
					if(n<0) lostconn(addr);
					//printf("sent EOL\n");
				}
				else{
					msg = "next\0";
					n = send(clt->sockfd,msg,sizeof(buffer),0);
					if(n<0) lostconn(addr);
					//printf("sent %d bytes to client\n",n);
				}
			}
		}
		
	}
}

void syserr(char *msg) { perror(msg); exit(-1); }
int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, n;
  struct sockaddr_in serv_addr, clt_addr;
  socklen_t addrlen;
  //Default to port 5000 if no port is given
  if(argc == 1) portno = 5000;
  else if(argc > 2) { 
    fprintf(stderr,"Usage: %s <port>\n", argv[0]);
    return 1;
  } 
  else portno = atoi(argv[1]); //converts portno string to int

	head = NULL;

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
for(;;){
  clt = malloc(sizeof(struct client));
  clt->sockfd = accept(sockfd, (struct sockaddr*)&(clt->clt_addr), &(clt->addrlen));
  clt->addrlen = sizeof(clt->clt_addr); 
  if(clt->sockfd < 0){
	close(clt->sockfd);	
 	printf("Cannot accept client\n");
  }
 else{
	 printf("New incoming connection...");
	 n = pthread_create(&(clt->clt_thread),NULL,&trackerLogic,clt);
  }
}
  close(sockfd); 
  return 0;
}
