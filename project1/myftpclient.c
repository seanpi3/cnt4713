#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/stat.h>

void syserr(char* msg) { perror(msg); exit(-1); }

int main(int argc, char* argv[])
{
  int sockfd, portno, n;
  struct hostent* server;
  struct sockaddr_in serv_addr;
  char buffer[256];
  char str[256];
  char *toke;
  char filename[256];
  char getStatus[256];
  if(argc != 3) {
    fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
    return 1;
  }
  server = gethostbyname(argv[1]);
  if(!server) {
    fprintf(stderr, "ERROR: no such host: %s\n", argv[1]);
    return 2;
  }
  portno = atoi(argv[2]);
  
  /*{
  struct in_addr **addr_list; int i;
  printf("Official name is: %s\n", server->h_name);
  printf("    IP addresses: ");
  addr_list = (struct in_addr **)server->h_addr_list;
  for(i = 0; addr_list[i] != NULL; i++) {
    printf("%s ", inet_ntoa(*addr_list[i]));
  }
  printf("\n");
  }*/

  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sockfd < 0) syserr("can't open socket");
  //printf("create socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
  serv_addr.sin_port = htons(portno);

  if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    syserr("can't connect to server");
  printf("Connection to server at %s:%s established. Now awaiting command...\n", argv[1], argv[2]);
  DIR *dir;
  struct dirent *ent;
for(;;){  
  printf("%s:%s> ",argv[1],argv[2]);
  memset(buffer,0,sizeof(buffer));
  fgets(buffer, sizeof(buffer), stdin);
  n=strlen(buffer);
  if(n>0 && buffer[n-1] == '\n') buffer[n-1]='\0';
  strcpy(str, buffer);
  toke = strtok(str, " ");

  //input conditions
  if(strcmp(toke,"ls-local")== 0){
	printf("File at the client:\n");
	dir = opendir(".");
	if(dir != NULL){
		while((ent = readdir(dir)) != NULL){
			if(strcmp(ent->d_name,".") && strcmp(ent->d_name,".."))	printf("%s\n",ent->d_name);
		}
	}
	closedir(dir);
  }
  else{
  	n = send(sockfd, buffer, sizeof(buffer), 0); 
 	if(n < 0){
  		printf ("Connection to the server %s:%s has been lost.", argv[1],argv[2]);
  		exit(0);
  	}
  	else if(strcmp(toke, "exit")==0 || strcmp(toke,"quit")==0 || strcmp(buffer,"stop")==0){
		printf("Connection to the server %s:%s terminated. Bye now!\n", argv[1],argv[2]);
		close(sockfd);
		exit(0);
  	}
  	else if(strcmp(toke,"get")==0){
		toke = strtok(NULL," ");
		strcpy(filename,toke);
		n = recv(sockfd, buffer ,sizeof(buffer),0);
		strcpy(getStatus,buffer);
		if(strcmp(getStatus,"succesful")==0){
			FILE *f = fopen(filename,"a");
			uint32_t sizeIn;
			n = recv(sockfd,&sizeIn,sizeof(uint32_t),0);
			uint32_t filesize = ntohl(sizeIn);
			int bytes_read,bytes_toRead, bytes_written;
			bytes_toRead = filesize;
			while(bytes_toRead > 0){
				bytes_read = read(sockfd,buffer,sizeof(buffer));
				if(bytes_toRead< sizeof(buffer)){
					bytes_written = fwrite(buffer, bytes_toRead,1,f);
				}
				else{
					bytes_written = fwrite(buffer,sizeof(buffer),1,f);
				}
				if(bytes_written <0) syserr("Error writing");
				bytes_toRead -= bytes_read;
			}
			fclose(f);
		}	
		else{
			printf("unsuccesful");
			memset(buffer, '\0',sizeof(buffer));
			n = recv(sockfd,buffer,sizeof(buffer),0);
			printf("%s\n",buffer);
		}
		printf("Retrieve file '%s' from server: %s\n",filename,getStatus);
  	}
	else if(strcmp(toke,"put")==0){
		toke = strtok(NULL, " " );
		int f = open(toke,0);
		int filesize;
  		struct stat st;
		stat(toke, &st);
		filesize = st.st_size;
		printf("Sending %d bits to server \n",filesize);
		if(f == -1){
			printf("Invalid file/format. Please use put <filename>\n");
		}
		else{
			uint32_t un = htonl((uint32_t)filesize);
			n = send(sockfd,&un,sizeof(uint32_t),0);
			int bytes_sent,bytes_read,bytes_remaining;
			bytes_remaining = filesize;
			while(bytes_remaining > 0 ){
				bytes_read = read(f,buffer,sizeof(buffer));
				bytes_sent = send(sockfd, buffer,sizeof(buffer),0);
				if(bytes_sent<0) syserr("error sending file");
				bytes_remaining -= bytes_sent; 
				printf("Transferring %d bits to server... %d remaining\n",bytes_sent,bytes_remaining);
			}
			printf("Finished sending file\n");
		}
		close(f);
	}
	else if(strcmp(toke,"ls-remote")==0){
		printf("Files at the server(%s):\n",argv[1]);
		n = recv(sockfd, buffer, sizeof(buffer), 0);
		if( n < 0 ) syserr("can't receive from server");
		printf("%s", buffer);
	}
  	else{
  		n = recv(sockfd, buffer,sizeof(buffer), 0);
  		if(n < 0) syserr("can't receive from server");
  		printf("%s\n", buffer);
  	}
  }
}
  close(sockfd);
  return 0;
}
