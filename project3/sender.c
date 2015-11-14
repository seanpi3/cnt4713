#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int windowSize = 100;
int packetSize = 1024;
int timeout = 10;

void syserr(char* msg) { perror(msg); exit(-1); }
int receiveACK(int sockfd){
	/*
	char* buff;
	int n = recv(sockfd,buff,3,0);
	if(n<=0) syserr("lost connection to receiver");
	if(strcmp(buff,"ACK"))	return -1;
	*/
}


uint16_t calculateChecksum(void* data,size_t length){
	char* d = (char*) data;
	uint16_t sum = 0;
	size_t i = 0;
	for(i;i+1<length;i+=2){
		uint16_t word;
		memcpy(&word,d+i,2);
		sum+=word;
	}	
	if(length&1){
		uint16_t word;
		memcpy(&word,d+length-1,1);
		sum+=word;
	}
	return sum;
}

int sendPacket(int sockfd,struct sockaddr* recv_addr, int seqNum, char[] data,size_t data_size){
	char packet[sizeof(int) + sizeof(uint16_t) + data_size];
	char payload[data_size];
	char snum[sizeof(int)];
	char cs[sizeof(int)];
	uint16_t checksum;
	payload = data;
	sprintf(snum,"%d",seqNum);
	checksum = calculateChecksum(snum,strlen(snum));
	checksum += calculateChecksum(data,data_size);
	checksum = 0xffff - checksum;
	checksum = htons(checksum);
	strcpy(packet,snum);
	strcat(packet, " ");
	sprintf(cs,"%d",checksum);
	strcat(packet,cs);
	strcat(packet, " ");
	strcat(packet,payload);
	printf("%s\n",packet);
	//n = sendto(sockfd, , strlen(buffer), 0, (struct sockaddr*)&recv_addr, addrlen);
}	

int main(int argc, char* argv[])
{
  int sockfd, portno, n;
  struct hostent* receiver;
  struct sockaddr_in recv_addr;
  socklen_t addrlen;
	int f;	
	char *filename;
	/*
  if(argc != 4) {
    fprintf(stderr, "Usage: %s <ip> <port> <file>\n", argv[0]);
    return 1;
  }
	*/
	//open file and check if it exists
	filename = argv[3];
	f = open(filename,0);
	if(f<=0) syserr("cannot open file: make sure it exists");
	/*
	//check if receiver is a valid host
  receiver = gethostbyname(argv[1]);
  if(!receiver) {
    fprintf(stderr, "ERROR: no such host: %s\n", argv[1]);
    return 2;
  }
  portno = atoi(argv[2]);
  
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(sockfd < 0) syserr("can't open socket");

  memset(&recv_addr, 0, sizeof(recv_addr));
  recv_addr.sin_family = AF_INET;
  recv_addr.sin_addr = *((struct in_addr*)receiver->h_addr);
  recv_addr.sin_port = htons(portno);
	addrlen = sizeof(recv_addr);


 // n = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr*)&recv_addr, &addrlen);

  close(sockfd);
	*/
	char buffer[packetSize];
	n = read(f,buffer,packetSize);
	//printf("%d\n",calculateChecksum(buffer,1024));
	sendPacket(1,NULL,5,buffer,packetSize);
  return 0;
}
