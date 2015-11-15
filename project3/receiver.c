#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

size_t packetSize = sizeof(int) + sizeof(uint16_t) + 1024;

void syserr(char *msg) { perror(msg); exit(-1); }

void printPacket(void* data,size_t length){
	char* d = (char*) data;
	size_t i;
	for(i=0;i+1<length;i+=2){
		uint16_t word;
		memcpy(&word,d+i,2);
		printf("%x",word);
	}
	if(length%2!=0){
		uint16_t word;
		memcpy(&word,d+length-1,1);
		printf("%x",word);
	}
	printf("\n");
}

uint16_t calculateChecksum(void* data,size_t length){
	char* d = (char*) data;
	uint16_t sum = 0;
	size_t i;
	for(i=0;i+1<length;i+=2){
		uint16_t word;
		memcpy(&word,d+i,2);
		sum+=word;
	}
	if(length%2!=0){
		uint16_t word;
		memcpy(&word,d+length-1,1);
		sum+=word;
	}
	return sum;
}

void sendACK(int seq_num, int sockfd,struct sockaddr *dest_addr,socklen_t addrlen){
	char *ACK[sizeof(uint32_t)+sizeof(uint16_t)+3];
	void *offset = ACK;
	uint32_t seqOut = htonl((uint32_t)seq_num);
	memcpy(offset,&seqOut,sizeof(uint32_t));
	char *msg = "ACK";
	uint16_t checkSum = calculateChecksum(&seqOut,sizeof(uint32_t));
	checkSum += calculateChecksum(msg,sizeof(msg));
	offset+=sizeof(uint32_t);
	memcpy(offset,&checkSum,sizeof(uint16_t));
	offset+=sizeof(uint16_t);
	memcpy(offset,msg,sizeof(msg));
  int n = sendto(sockfd,ACK,sizeof(ACK), 0, (struct sockaddr*)&dest_addr, addrlen);
}

int receivePacket(int sockfd,void* buffer,size_t length,struct sockaddr* sdr_addr,socklen_t* addrlen){
	int n = 0,remaining = length;
	void *offset = buffer;
	while(remaining>0){
		offset+=packetSize-remaining;
 	 	n = recvfrom(sockfd,offset,packetSize, 0,sdr_addr, &addrlen); 
  	if(n < 0) return -1;
		remaining-=n;
	}
	uint16_t check = calculateChecksum(buffer,length);
	if(check==0xffff){
		printf("Checksum verified... sending ACK.\n");
		return 1;
	}
	else{
		printf("Data corrupted rerequesting packet.\n"); 
		return 0;
	}
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n;
  struct sockaddr_in serv_addr, sdr_addr; 
  socklen_t addrlen;
	char packet[sizeof(uint32_t)+sizeof(uint16_t)+1024];

  if(argc != 3) { 
    fprintf(stderr,"Usage: %s <port> <filename>\n", argv[0]);
    return 1;
  } 
  portno = atoi(argv[1]);

  sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
  if(sockfd < 0) syserr("can't open socket"); 

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    syserr("can't bind");
  addrlen = sizeof(serv_addr); 
	memset(packet,0,sizeof(packet));
	n = receivePacket(sockfd,packet,sizeof(packet),(struct sockaddr*)&serv_addr,addrlen);
	if(n<0) syserr ("failed to receive packet\n");
	
	void *offset = packet;
	uint32_t seq_num;
	uint32_t seq_num_in;
	memcpy(&seq_num_in,offset,sizeof(uint32_t));
	seq_num = ntohl(seq_num_in);
	offset+=sizeof(uint32_t);
	offset+=sizeof(uint16_t);
	uint32_t sizeIn;
	memcpy(&sizeIn,offset,sizeof(uint32_t));
	uint32_t fileSize = ntohl(sizeIn);
	//char payload[1024];
	//memcpy(payload,offset,1024);
  //if(n < 0) syserr("can't send to server"); 
	printf("Packet received.\nseqnum:%d, filesize: %d\n",seq_num,fileSize);
	sendACK((int)seq_num,sockfd,(struct sockaddr*)&serv_addr,addrlen);
  close(sockfd); 
  return 0;
}
