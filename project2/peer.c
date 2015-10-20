#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

int trackersockfd, serversockfd, serverport;
struct sockaddr_in serv_addr;
typedef struct fileList{
	char *filename;
	char *ip;
	int port;
	struct fileList *nextFile;
};
struct fileList *head;
struct fileList *tail;

void syserr(char* msg) { perror(msg); exit(-1); }

int receiveAll(int sock,char *buffer, int bytes){
		int n=0;
		int bytes_Received= 0;
		int bytes_toReceive = bytes;
		char *offset = buffer;
		while(bytes_toReceive > 0){
			n = recv(sock,offset,bytes_toReceive,0);
			if(n<=0) break;
			bytes_toReceive-=n;
			bytes_Received+=n;
			offset += n;
		}
		//printf("Read %d bytes\n",bytes_Received);
		return bytes_Received; 
}

struct fileList* fileAt(int i){
	int x;
	struct fileList *current = head;
	for(x=0;x<i;x++){
		current = current->nextFile;
		if(current==NULL) return NULL;
	}
	return current;
}
//Logic for the server thread
void *serverLogic(void *arg){
	
  int clientsockfd,n;
  struct sockaddr_in clt_addr;
  socklen_t addrlen;
  if(bind(serversockfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
    close(serversockfd);
    syserr("can't bind");
  }
  listen(serversockfd,5);
  for(;;){
    clientsockfd = accept(serversockfd, (struct sockaddr*)&clt_addr,&addrlen);
    addrlen = sizeof(clt_addr);
    if(clientsockfd <0){
	close(clientsockfd);
	printf("Cannot accept client\n");
    }
    else{
	//printf("peer connected...\n");
	int pid = fork();
	if(pid<0) syserr("error");
	else if (pid ==0) close(clientsockfd);
	else{
	  char buffer[256];
	  char *filename;
	  char *toke;
	  int filesize;
	  struct stat st;
	  filename = malloc(sizeof(buffer));
	    memset(buffer,0,sizeof(buffer));
	    n = recv(clientsockfd,buffer,sizeof(buffer),0);
	    if(n<0) {
			//printf("Peer lost connection\n");
			close(clientsockfd);
			pthread_exit(NULL);
	    }
		strcpy(filename, buffer);
		int f = open(filename,0);
		stat(filename,&st);
		filesize = st.st_size;
		//printf("Sending %d bytes to peer\n",filesize);
		if(f==-1){
			printf("error\n");
		}
		else{
			uint32_t un = htonl((uint32_t)filesize);
			n = send(clientsockfd,&un,sizeof(uint32_t),0);
			if(n<0) printf("errorhere");
			int bytes_sent,bytes_read, bytes_remaining,bytes_to_send;
			bytes_remaining = filesize;
			while(bytes_remaining >0 ){
				if(bytes_remaining<sizeof(buffer)){
					bytes_read = read(f,buffer,bytes_remaining);
					if(bytes_read<0) printf("error\n");
					bytes_sent = send(clientsockfd, buffer, bytes_remaining,0);
					if(bytes_sent < 0 ) printf("error\n");
				}
				else{
					bytes_read = read(f,buffer,sizeof(buffer));
					if(bytes_read<0)printf("error\n");
					bytes_sent = send(clientsockfd,buffer,sizeof(buffer),0);
					if(bytes_sent<0)printf("error\n");
				}
				bytes_remaining -= bytes_sent;
			}
		//printf("Finished sending file\n");

		}
		close(f);
		pthread_exit(NULL);
	  }
    }
  }
  

  
  
}

int main(int argc, char* argv[])
{
  int peersockfd,trackerport,peerport, n;
  struct hostent* tracker, *peer;
  struct sockaddr_in tracker_addr, peer_addr;
  socklen_t addrlen;
  pthread_t servt;
  char buffer[256];
	char *msg;

  if(argc != 4) {
    fprintf(stderr, "Usage: %s <tracker-hostname> <tracker-port> <local-port>\n", argv[0]);
    return 1;
  }
  tracker = gethostbyname(argv[1]);
  if(!tracker) {
    fprintf(stderr, "ERROR: no such host: %s\n", argv[1]);
    return 2;
  }
  trackerport = atoi(argv[2]);
  serverport = atoi(argv[3]);
  /*{
  struct in_addr **addr_list; int i;
  printf("Official name is: %s\n", tracker->h_name);
  printf("    IP addresses: ");
  addr_list = (struct in_addr **)tracker->h_addr_list;
  for(i = 0; addr_list[i] != NULL; i++) {
    printf("%s ", inet_ntoa(*addr_list[i]));
  }
  printf("\n");
  }*/
  
  //Set up tracker and server sockets
  trackersockfd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
  if(trackersockfd < 0) syserr("can't open socket");
  serversockfd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
  if(serversockfd < 0) syserr("can't open socket");
  
  //Set up tracker address
  memset(&tracker_addr, 0, sizeof(tracker_addr));
  tracker_addr.sin_family = AF_INET;
  tracker_addr.sin_addr = *((struct in_addr*)tracker->h_addr);
  tracker_addr.sin_port = htons(trackerport);

  //Set up server address
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(serverport);
  
  //Create thread for the server
  n = pthread_create(&servt,NULL, &serverLogic, NULL);
  if(n<0) syserr("can't create thread");
  
  //Connect to tracker  
  if(connect(trackersockfd, (struct sockaddr*)&tracker_addr, sizeof(tracker_addr)) < 0)
    syserr("can't connect to tracker");
  printf("Connected to tracker.\n");
	
	//Initial communication with tracker

	memset(buffer,0,sizeof(buffer));
	n = recv(trackersockfd,buffer,sizeof(buffer),0);
	if(n<0) syserr("lost connection with tracker");
	//printf("Received %d bytes from tracker\n",n);
  if(strcmp(buffer,"reqport") == 0){
		uint32_t portOut = htonl((uint32_t)serverport);
		n = send(trackersockfd,&portOut,sizeof(uint32_t),0);
		if(n<0) syserr("lost connection with tracker");
		//printf("Sent %d bytes to tracker\n",n);
	}
	else syserr("unable to properly communicate with tracker");
	memset(buffer,0,sizeof(buffer));
	n = recv(trackersockfd,buffer,sizeof(buffer),0);
	if(n<0) syserr("lost connection with tracker");
	//printf("Received %d bytes from tracker\n",n);
  if(strcmp(buffer,"reqfiles") ==0){
		DIR *dir;
		struct dirent *ent;
		dir = opendir(".");
		int numfiles = -2;
		while((ent = readdir(dir)) != NULL) numfiles ++;
		uint32_t fileCount = htonl((uint32_t)numfiles);
		n = send(trackersockfd, &fileCount,sizeof(uint32_t),0);
		if(n<0) syserr("lost connection with tracker");
		closedir(dir);
		dir = opendir(".");
		while((ent = readdir(dir)) != NULL){
			if(strcmp(ent->d_name, ".") && strcmp(ent->d_name,"..")){
				msg = ent->d_name;
				n = send(trackersockfd,msg,sizeof(buffer),0);
				if(n<0) syserr("lost connection with tracker");
				//printf("send %s :%d byes\n",msg,n);
			}
		}
		closedir(dir);
	}
	else syserr("unable to properly communicate with tracker");

	//test list
	/*
	head = malloc(sizeof(struct fileList));
	int g=0;
	struct fileList *derp = head; 
	head->filename = "head";
	head->nextFile = NULL;
	
	for(g=0;g<20;g++){
			derp->nextFile = malloc(sizeof(struct fileList));
			derp = derp->nextFile;
			derp->filename = "test";
			derp->ip="localhost";
			derp->port = g;
			derp->nextFile = NULL;
	}
	//test list
	*/

	//Main program loop
	char *toke;
	char *command;
	command = malloc(sizeof(buffer));
	head =malloc(sizeof(struct fileList));
	head->filename = malloc(sizeof(buffer));
	head->ip = malloc(sizeof(buffer));
	head->nextFile = NULL;
	for(;;){
		memset(buffer,0,sizeof(buffer));
		printf("> ");
		fgets(buffer,sizeof(buffer),stdin);
		while(strcmp(buffer,"")==0){
			fgets(buffer,sizeof(buffer),stdin);
		}
		n = strlen(buffer);
		if(n>0 && buffer[n-1] == '\n') buffer[n-1]='\0';
		strcpy(command, buffer);
		toke = strtok(command," ");
		memset(buffer,0,sizeof(buffer));
		if(strcmp(toke,"list")==0){
			msg = "reqlist\0";
			n = send(trackersockfd,msg,sizeof(buffer),0);
			if(n<0) syserr("lost connection to tracker");
			//printf("sent %d bytes to server\n",n);
			struct fileList *current = head;
			int count = 0;
			while(strcmp(buffer,"EOL")){
				memset(buffer,0,sizeof(buffer));
				receiveAll(trackersockfd,buffer,sizeof(buffer));
				//n = recv(trackersockfd,buffer,sizeof(buffer),0);
				//if(n<=0) syserr("lost connection to tracker");
				printf("[%d] %s ",count, buffer);
				//printf(": %d bytes from tracker\n",n);
				strcpy(current->filename,buffer);
				memset(buffer,0,sizeof(buffer));
				receiveAll(trackersockfd,buffer,sizeof(buffer));
				//n = recv(trackersockfd,buffer,sizeof(buffer),0);
				if(n<=0) syserr("lost connection to tracker");
				strcpy(current->ip,buffer);
				printf("%s:", buffer);//printf("%s:",current->ip);
				//printf("received: %d bytes from tracker\n",n);
				uint32_t portIn;
				memset(buffer,0,sizeof(buffer));
				n = recv(trackersockfd,&portIn,sizeof(uint32_t),0);
				if(n<=0) syserr("lost connection to tracker");
				uint32_t peerPort = ntohl(portIn);
				printf("%d\n",peerPort);
				//printf("received: %d bytes from tracker\n",n);
				memset(buffer,0,sizeof(buffer));
				receiveAll(trackersockfd,buffer,sizeof(buffer));
				//n = recv(trackersockfd,buffer,sizeof(buffer),0);
				//buffer[n] = '\0';
				//printf("received %s: %d bytes from tracker\n",buffer,n);
				//if(n<=0) syserr("lost connection to tracker");
				current->port = peerPort;
				if(strcmp(buffer,"EOL") && current->nextFile==NULL){
				   	current->nextFile = malloc(sizeof(struct fileList));					current = current->nextFile;
					current->filename = malloc(sizeof(buffer));
					current->ip = malloc(sizeof(buffer));
				}
				else current = current->nextFile;
				count++;
			}
		}
		else if(strcmp(toke,"download")==0){
			toke = strtok(NULL," ");
			int i = atoi(toke);
			struct fileList *selected = fileAt(i);
			printf("Downloading %s from peer %s:%d\n",selected->filename,selected->ip,selected->port);
			
			peersockfd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
			if(peersockfd < 0 ) printf("ERROR: Can't open socket");
			peer = gethostbyname(selected->ip);
			if(!peer) printf("Cannot find clinet");

			//Set up peer address
			memset(&peer_addr,0,sizeof(peer_addr));
			peer_addr.sin_family = AF_INET;
			peer_addr.sin_addr = *((struct in_addr*)peer->h_addr);
			peer_addr.sin_port = htons(selected->port);

			if(connect(peersockfd,(struct sockaddr*)&peer_addr,sizeof(peer_addr)) < 0){
				printf("can't connect to client.\n");
				}
			
			n = send(peersockfd,selected->filename,sizeof(buffer),0);
			if(n<=0) printf("error\n");
			memset(buffer,0,sizeof(buffer));

			FILE *f = fopen(selected->filename,"wb");
			uint32_t sizeIn;
			n = recv(peersockfd,&sizeIn,sizeof(uint32_t),0);
			if(n<=0) printf("error1\n");
			uint32_t filesize = ntohl(sizeIn);
			int bytes_read,bytes_toRead,bytes_written;
			bytes_toRead = filesize;
			printf("Receiving %d bytes from peer...\n", bytes_toRead);
			while(bytes_toRead > 0){
				memset(buffer,0,sizeof(buffer));
				//bytes_read = read(peersockfd,buffer,sizeof(buffer));
				if(bytes_toRead < 256){
					bytes_read = receiveAll(peersockfd,buffer,bytes_toRead);
					bytes_written = fwrite(buffer,bytes_toRead,1,f);
				}
				else{
					bytes_read = receiveAll(peersockfd,buffer,sizeof(buffer));
					bytes_written = fwrite(buffer,sizeof(buffer),1,f);

				}
				if(bytes_written <0) printf("error3\n");
				bytes_toRead -= bytes_read;
			}
			fclose(f);
			close(peersockfd);
			printf("%s downloaded successully.\n",selected->filename);
	
		}
		else if(strcmp(toke,"exit")==0){
			close(trackersockfd);
			printf("Connection to the tracker %s:%d terminated.\n",argv[1],argv[2]);
			close(serversockfd);
			pthread_kill(servt);
			printf("File server at port %d terminated.\nBye now!\n",argv[3]);
			return 1;
		}
		else if(strcmp(toke,"print")==0){
			struct fileList *current = head;
			while(current != NULL){
				printf("%s:%d\n",current->filename,current->port);
				current = current->nextFile;
			}
		}
	
	}
  
  /*
  printf("PLEASE ENTER MESSAGE: ");
  fgets(buffer, 255, stdin);
  n = strlen(buffer); if(n>0 && buffer[n-1] == '\n') buffer[n-1] = '\0';

  n = send(sockfd, buffer, strlen(buffer), 0);
  if(n < 0) syserr("can't send to server");
  printf("send...\n");
  
  n = recv(sockfd, buffer, 255, 0);
  if(n < 0) syserr("can't receive from server");
  else buffer[n] = '\0';
  printf("CLIENT RECEIVED MESSAGE: %s\n", buffer);
  */
  close(trackersockfd);
  close(serversockfd);
  return 0;
}
