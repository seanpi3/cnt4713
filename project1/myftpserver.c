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
 	printf("Cannot accept client\n");
  }
  else printf("Client connected...\n");
  int pid = fork();
  if(pid<0) syserr("Error");
  else if(pid == 0) {
	close(newsockfd);
  }
  else{	
  	char buffer[256];
	char command[256];
	char *msg;
	DIR *dir;
	struct dirent *ent;
	char *toke;  
    for(;;){
  	n = recv(newsockfd, buffer, sizeof(buffer), 0); 
  	if(n < 0) syserr("can't receive from client"); 
	else buffer[n] = '\0';
	printf("SERVER GOT MESSAGE: %s\n", buffer); 
    	toke = strtok(buffer," ");
	if(strcmp(toke,"stop")==0){
		msg = "Server stopping.\n";
		n = send(newsockfd, msg, strlen(msg),0);
		close(newsockfd);
		close(sockfd);
		printf("server stopping\n");
		exit(0);
	}
	else if(strcmp(toke,"ls-remote")==0){
		dir = opendir(".");
		if(dir != NULL){
			while((ent = readdir(dir)) != NULL){
				msg = ent->d_name;
				n = send(newsockfd,msg,strlen(msg),0);
				printf("SENT:%s\n",msg);
				msg = "\n";
				n = send(newsockfd,msg,strlen(msg),0);	
				printf("SENT:%s\n",msg);
			}
		}
		else{
			printf("null directory\n");
		}
		closedir(dir);
		printf("ls-remote\n");
	}
	else if(strcmp(toke,"get")==0){
		toke = strtok(NULL ,"\0");
		FILE *f = fopen(toke,"rb" );
		msg = "succesful";
		if (f == NULL){
			msg = "failed";
			n = send(newsockfd,msg,sizeof(buffer),0);
			printf("SENT:%s\n",msg);
			msg = "Invalid file/format. Please use get <filename>\0";
			n = send(newsockfd,msg,sizeof(buffer),0);
			printf("SENT:%s\n",msg);
		}
		else{
			n = send(newsockfd,msg,sizeof(buffer),0);
			printf("SENT:%s\n",msg);
			memset(buffer, '\0', sizeof(buffer));
			n = fread(buffer,sizeof(buffer),1,f);
			if(n < 0) printf("Error reading file");
			n = send(newsockfd, buffer, sizeof(buffer),0);
			printf("SENT:%s\n",buffer);
			fclose(f);
		}
	}
	else if(strcmp(toke,"put")==0){
		msg = "putting\0";
		n = send(newsockfd,msg,sizeof(buffer),0);
		printf("SENT:%s\n",msg);
	}
	else{
		msg = "Invalid command. Please send one of the following valid commands: ls-remote, get <filename>, put <filename>, stop.\0";
		n = send(newsockfd, msg,sizeof(buffer),0);
		printf("SENT:%s\n",msg);
	}
    }
  	close(newsockfd); 
  }
}
  close(sockfd); 
  return 0;
}
