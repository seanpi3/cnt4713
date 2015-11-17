#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

size_t packetSize = sizeof(int) + sizeof(uint16_t) + 1000;
int *receivedPackets;
int rfp=0;

typedef struct filepackets{
	int seq_num;
	struct filepackets *next;
};
struct filepackets *head;
struct filepackets *tail;

int alreadyReceived(int seq_num){
	struct filepackets *current = tail;
	while(current!=NULL){
		if(current->seq_num==seq_num) return 1;
		current=current->next;
	}
	return 0;
}

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

int sendACK(int seq_num, int sockfd,struct sockaddr *dest_addr,socklen_t addrlen){
	char *ACK[sizeof(uint32_t)+sizeof(uint16_t)+3];
	void *offset = ACK;
	uint32_t seqOut = htonl((uint32_t)seq_num);
	memcpy(offset,&seqOut,sizeof(uint32_t));
	char *msg = "ACK";
	uint16_t checkSum = calculateChecksum(&seqOut,sizeof(uint32_t));
	checkSum += calculateChecksum(msg,3);
	checkSum = 0xffff - checkSum;
	offset+=sizeof(uint32_t);
	memcpy(offset,&checkSum,sizeof(uint16_t));
	offset+=sizeof(uint16_t);
	memcpy(offset,msg,3);
  int n = sendto(sockfd,ACK,sizeof(ACK),0,dest_addr,addrlen);
	return n;
}

int receivePacket(int sockfd,void* buffer,size_t length,struct sockaddr* sdr_addr,socklen_t* addrlen){
	int n = 0,remaining = length;
	void *offset = buffer;
	while(remaining>0){
		offset+=packetSize-remaining;
 	 	n = recvfrom(sockfd,offset,length,0,sdr_addr, &addrlen); 
  	if(n < 0) return -1;
		remaining-=n;
	}
	uint16_t check = calculateChecksum(buffer,length);
	if(check==0xffff){
		//printf("Checksum verified... sending ACK.\n");
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
	int payloadSize = 1000;
	char packet[packetSize];
	char *filename;
	FILE *f;
	void *file;

  if(argc != 3) { 
    fprintf(stderr,"Usage: %s <port> <filename>\n", argv[0]);
    return 1;
  } 
	filename = argv[2];
	f = fopen(filename,"wb");
	if(f==NULL) syserr("cannot open file for writing");

  portno = atoi(argv[1]);

  sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
  if(sockfd < 0) syserr("can't open socket"); 

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    syserr("can't bind");
  addrlen = sizeof(sdr_addr); 

	int numPackets;
	int packetsReceived = 0;
	int bytes_remaining;
	uint32_t fileSize;
	fd_set set;
	struct timeval tv;
	FD_ZERO(&set);
	FD_SET(sockfd,&set);
	n = select(sockfd+1,&set,NULL,NULL,NULL);
	head = malloc(sizeof(struct filepackets));
	head->next = NULL;
	tail = head;
	while(1){
		memset(packet,0,sizeof(packet));
		FD_ZERO(&set);
		FD_SET(sockfd,&set);
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		n = select(sockfd+1,&set,NULL,NULL,&tv);
		if(n==0) break;
		n = receivePacket(sockfd,packet,sizeof(packet),(struct sockaddr*)&sdr_addr,addrlen);
		if(n<0) syserr ("failed to receive packet\n");
		if(n>0){
			void *offset = packet;
			uint32_t seq_num;
			uint32_t seq_num_in;
			memcpy(&seq_num_in,offset,sizeof(uint32_t));
			seq_num = ntohl(seq_num_in);
			int alr_recvd;
			if(!rfp && seq_num == 0){
				offset+=sizeof(uint32_t);
				offset+=sizeof(uint16_t);
				uint32_t sizeIn;
				memcpy(&sizeIn,offset,sizeof(uint32_t));
				fileSize = ntohl(sizeIn);
				bytes_remaining = fileSize;
				FD_ZERO(&set);
				FD_SET(sockfd,&set);
				n = select(sockfd+1,NULL,&set,NULL,NULL);
				sendACK((int)seq_num,sockfd,(struct sockaddr*)&sdr_addr,addrlen);
				numPackets = fileSize/payloadSize;
				if(fileSize%payloadSize!=0) numPackets++;
				printf("filesize: %d\nexpecting %d packets\n",fileSize,numPackets);
				packetsReceived++;
				file = malloc(fileSize);
				receivedPackets = malloc(sizeof(int)*numPackets);
				memset(receivedPackets,0,sizeof(int)*numPackets);
				receivedPackets[seq_num] = seq_num;
				head->seq_num = seq_num;
				rfp = 1;
			}
			else if(!alreadyReceived(seq_num)){
				head->next = malloc(sizeof(struct filepackets));
				head = head->next;
				head->seq_num = seq_num;
				offset+=sizeof(uint32_t);
				offset+=sizeof(uint16_t);
				char payload[payloadSize];
				memcpy(payload,offset,payloadSize);
				offset = file;
				offset+=((seq_num-1)*payloadSize);
				if(bytes_remaining < payloadSize){
					memcpy(offset,payload,bytes_remaining);
					bytes_remaining-=bytes_remaining;
				}
				else{
					memcpy(offset,payload,payloadSize);
					bytes_remaining-=payloadSize;
				}
				FD_ZERO(&set);
				FD_SET(sockfd,&set);
				n = select(sockfd+1,NULL,&set,NULL,NULL);
				n = sendACK((int)seq_num,sockfd,(struct sockaddr*)&sdr_addr,addrlen);
				if(n<0) printf("ERROR\n");
				if(n==0) printf("ERROR\n");
				receivedPackets[seq_num] == seq_num;
				packetsReceived++;
				//printf("Received packet %d\n",seq_num);
				//printf("%d bytes remaining to be received\n",bytes_remaining);
				if(packetsReceived%1000==0) printf("%d packets received\n",packetsReceived);
			}
			else{
				FD_ZERO(&set);
				FD_SET(sockfd,&set);
				n = select(sockfd+1,NULL,&set,NULL,NULL);
				n = sendACK((int)seq_num,sockfd,(struct sockaddr*)&sdr_addr,addrlen);
			}
		}
	}
	printf("filesize: %d\nexpecting %d packets\n",fileSize,numPackets);
	printf("Transmission complete... %d packets received.\n",packetsReceived);
	n = fwrite(file,fileSize,1,f);
	if(n<0) syserr("cannot write file");
	fclose(f);
  close(sockfd); 
  return 0;
}
