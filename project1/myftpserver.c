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

void syserr(char *msg) { perror(msg); exit(-1); }
void lostconn() {
	printf("Client lost connection");
	pthread_exit();
}


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
 else{
  printf("Client connected...\n");
  int pid = fork();
  if(pid<0) syserr("Error");
  else if(pid == 0) {
	close(newsockfd);
  }
  else{	
  	char buffer[256];
	char command[256];
	char filename[256];
	char *msg;
	DIR *dir;
	struct dirent *ent;
	char *toke;  
	int filesize;
	struct stat st;
    for(;;){
	memset(buffer,0,sizeof(buffer));
  	n = recv(newsockfd, buffer, sizeof(buffer), 0); 
  	if(n < 0) lostconn();
	else buffer[n] = '\0';
	printf("SERVER GOT MESSAGE: %s\n", buffer); 
    	toke = strtok(buffer," ");
	if(strcmp(toke,"stop")==0){
		msg = "Server stopping.\n";
		n = send(newsockfd, msg, strlen(msg),0);
		if(n<0) lostconn();
		close(newsockfd);
		close(sockfd);
		printf("server stopping\n");
		exit(0);
	}
	else if(strcmp(toke,"exit")==0){
		close(newsockfd);
		pthread_exit();
	}
	else if(strcmp(toke,"ls-remote")==0){
		dir = opendir(".");
		if(dir != NULL){
			int numfiles = -2;
			while((ent = readdir(dir)) != NULL){
				numfiles++;
			}
			uint32_t un = htonl((uint32_t)numfiles);
			n = send(newsockfd,&un,sizeof(uint32_t),0);
			if(n<0) lostconn();
			dir = opendir(".");
			while((ent = readdir(dir)) != NULL){
				if(strcmp(ent->d_name, ".") && strcmp(ent->d_name,"..") ){
					msg = ent->d_name;
					n = send(newsockfd,msg,sizeof(buffer),0);
					if(n<0) lostconn();
				}
			}
		}
		else{
			printf("null directory\n");
		}
		closedir(dir);
	}
	else if(strcmp(toke,"get")==0){
		toke = strtok(NULL ,"\0");
		int f = open(toke,0);
		stat (toke, &st);
		filesize = st.st_size;
		printf("Sending %d bits to client \n",filesize);
		msg = "succesful";
		if (f == -1 ){
			msg = "failed";
			n = send(newsockfd,msg,sizeof(buffer),0);
			if(n<0) lostconn();
			msg = "Invalid file/format. Please use get <filename>\0";
			n = send(newsockfd,msg,sizeof(buffer),0);
			if(n<0) lostconn();
			printf("SENT:%s\n",msg);
		}
		else{
			n = send(newsockfd,msg,sizeof(buffer),0);
			if(n<0) lostconn();
			printf("SENT:%s\n",msg);
			uint32_t un = htonl((uint32_t)filesize);
			n = send(newsockfd,&un,sizeof(uint32_t),0);
			if(n<0) lostconn();
			int bytes_sent,bytes_read, bytes_remaining, bytes_to_send;
			bytes_remaining = filesize;
			while(bytes_remaining > 0){
				bytes_read = read(f,buffer,sizeof(buffer));
				if(bytes_read < 0) lostconn();
				bytes_sent = send(newsockfd, buffer,sizeof(buffer),0);
				if(bytes_sent < 0) lostconn();
				bytes_remaining -= bytes_sent;
				printf("Transferring %d bits to client... %d remaining\n",bytes_sent,bytes_remaining);
			}
			printf("Finished sending file\n");
		}
			close(f);
		
	}
	else if(strcmp(toke,"put")==0){
		toke = strtok(NULL," ");
		strcpy(filename,toke);
		FILE *f = fopen(filename,"a");
		uint32_t sizeIn;
		n = recv(newsockfd,&sizeIn, sizeof(uint32_t),0);
		if(n<0) lostconn();
		uint32_t filesize = ntohl(sizeIn);
		int bytes_read,bytes_toRead,bytes_written;
		bytes_toRead = filesize;
		printf("Receiving %d bits from client\n",filesize);
		while(bytes_toRead > 0){
			bytes_read = read(newsockfd,buffer,sizeof(buffer));
			if(bytes_read <0) lostconn();
			if(bytes_toRead<sizeof(buffer)){
				bytes_written = fwrite(buffer,bytes_toRead,1,f);
				if(bytes_written<0) lostconn();
			}
			else{
				bytes_written = fwrite(buffer,sizeof(buffer),1,f);
				if(bytes_written < 0) lostconn();
			}
			bytes_toRead -= bytes_read;
			if(bytes_written<0) syserr("Error writing");
			printf("Receiving %d bits from client... %d remaining\n",bytes_read, bytes_toRead);
		}
		fclose(f);
		printf("Received file '%s' from client\n",filename);
	}
	else{
		msg = "Invalid command. Please send one of the following valid commands: ls-remote, get <filename>, put <filename>, stop.\0";
		n = send(newsockfd, msg,sizeof(buffer),0);
		if(n<0) lostconn();
		printf("SENT:%s\n",msg);
	}
    }
  	close(newsockfd); 
  }
 }
}
  close(sockfd); 
  return 0;
}
