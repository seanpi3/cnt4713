#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void syserr(char* msg) { perror(msg); exit(-1); }

int main(int argc, char* argv[])
{
  int sockfd, portno, n;
  struct hostent* server;
  struct sockaddr_in serv_addr;
  char buffer[256];
  char str[256];
  char *toke;
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
for(;;){  
  printf("%s:%s> ",argv[1],argv[2]);
  fgets(buffer, 255, stdin);
  n = strlen(buffer); 
  if(n>0 && buffer[n-1] == '\n') buffer[n-1] = '\0';

  n = send(sockfd, buffer, strlen(buffer), 0);
  if(n < 0){
  	printf ("Connection to the server %s:%s has been lost.", argv[1],argv[2]);
  	exit(0);
  }
  toke = strtok(buffer, " ");
  if(strcmp(toke,"quit")==0 || strcmp(buffer,"stop")==0){
 	printf("Exit command recieved\n");
	close(sockfd);
	exit(0);
   }
  else if(strcmp(toke,"get")==0){
	FILE *f;
	toke = strtok(NULL," ");
	strcpy(str,toke);
	n = recv(sockfd, buffer , 255, 0);
	printf("Retrieve file '%s' from server: %s\n", str,buffer);
	if(strcmp(buffer,"succesful")==0){
		f = fopen("derpt","w");
		n = recv(sockfd, buffer, 255, 0);
		if(f == NULL) printf("Could not write received file.");
		fwrite(buffer,sizeof(buffer),1,f);
		fclose(f);
	}
	else{
		memset(buffer, '\0',sizeof(buffer));
		n = recv(sockfd,buffer,255,0);
		printf("%s\n",buffer);
	}
  }
  else{
  	n = recv(sockfd, buffer, 255, 0);
  	if(n < 0) syserr("can't receive from server");
  	else buffer[n] = '\0';
  	printf("Server says: %s\n", buffer);
  }
}
  close(sockfd);
  return 0;
}
