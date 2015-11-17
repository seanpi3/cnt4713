#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>

int sockfd;
int windowSize = 100;
int payloadSize = 1000;
int to = 10;
struct sockaddr_in recv_addr;
socklen_t addrlen;

typedef struct window{
	int seq_num;
	void *payload;
	int receivedACK;
	pthread_t packet_thread;
	struct window *next;
	int ready_to_send;
};

struct window *head;
struct window *tail;

void syserr(char* msg) { perror(msg); exit(-1); }

struct window* findPacket(int seq_num){
	struct window *current = tail;
	while(current!=NULL){
		if(current->seq_num==seq_num){
			return current;
		}
		current=current->next;
	}
	return NULL;
}


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

int receiveACK(int sockfd,void* buffer,size_t length,struct sockaddr* sdr_addr,socklen_t* addrlen){
	int n = 0, remaining = length;
	void *offset = buffer;
	while(remaining>0){
		offset+=length-remaining;
		n = recvfrom(sockfd,offset,remaining,0,sdr_addr,&addrlen);
		if(n<0) return -1;
		remaining-=n;
	}
	uint16_t check = calculateChecksum(buffer,length);
	if(check==0xffff){
		//printf("Checksum verified.. ACK received.\n");
		return 1;
	}
	else{
		printf("Data corrupted... ACK not received.\n");
		return 0;
	}
}

int sendPacket(int seqNum,void *data,size_t data_size, int sockfd,struct sockaddr *dest_addr,socklen_t dest_len){
	char packet[sizeof(uint32_t)+sizeof(uint16_t)+data_size];
	uint16_t checksum;
	void *offset = packet;
	uint32_t seqNumOut = htonl((uint32_t)seqNum);
	memset(packet,0,sizeof(packet));
	checksum = calculateChecksum(&seqNumOut,sizeof(uint32_t));
	checksum += calculateChecksum(data,data_size);
	checksum = 0xffff - checksum;
	//checksum = htons(checksum);
	memcpy(offset,&seqNumOut,sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(offset,&checksum,sizeof(uint16_t));
	offset+=sizeof(uint16_t);
	memcpy(offset,data,data_size);
	//printPacket(packet,sizeof(packet));
	int n = sendto(sockfd,packet,sizeof(packet),0,dest_addr,dest_len);
	return 1;
}	

void *ackLogic(void *arg){
	char ACK[sizeof(uint32_t)+sizeof(uint16_t)+3];
	void *offset;
	fd_set readset;
	int n;
	uint32_t seq_num,seq_num_in;
	uint16_t checkSum;
	while(1){
		memset(ACK,0,sizeof(ACK));
		FD_ZERO(&readset);
		FD_SET(sockfd,&readset);
		n = select(sockfd + 1,&readset,NULL,NULL,NULL);
	//printf("ACK arrived\n");
		n = receiveACK(sockfd,ACK,sizeof(ACK),(struct sockaddr*)&recv_addr,addrlen);
		if(n>0){
			offset = ACK;
			memcpy(&seq_num_in,offset,sizeof(uint32_t));
			seq_num = ntohl(seq_num_in);
			struct window *packet = findPacket((int)seq_num);
			packet->receivedACK = 1;
			//printf("Ack received for packet %d\n",packet->seq_num);
		}
		if(n==0) printf("ERROR\n");
		if(n<0) printf("ERROR\n");
	}
}

void *packetLogic(void *arg){
		struct window *packet = (struct window *)arg;
		packet->receivedACK = 0;
		fd_set set;
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = to * 1000;
		int n;
		do{
				//printf("Sending packet %d\n", packet->seq_num);
				FD_ZERO(&set);
				FD_SET(sockfd,&set);
				n = select(sockfd+1,NULL,&set,NULL,NULL);
				n = sendPacket(packet->seq_num,packet->payload,
									payloadSize,sockfd,(struct sockaddr*)&recv_addr,addrlen);
				//n = select(0,NULL,NULL,NULL,&timeout);
			
				int i;
				timeout.tv_usec = 1000;
				for(i=0;i<10;i++){
					n = select(0,NULL,NULL,NULL,&timeout);
					if(packet->receivedACK) break;
				}
				
		} while(!packet->receivedACK);
		//printf("ACK received for packet %d\n",packet->seq_num);
}

int main(int argc, char* argv[])
{
  int portno, n;
  struct hostent* receiver;
	FILE *f;	
	char *filename;
	
  if(argc != 4) {
    fprintf(stderr, "Usage: %s <ip> <port> <file>\n", argv[0]);
    return 1;
  }
	
	//open file and check if it exists
	filename = argv[3];
	f = fopen(filename,"rb");
	if(f==NULL) syserr("cannot open file: make sure it exists");
	
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

	
	time_t start_time = time(NULL);
 // n = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr*)&recv_addr, &addrlen);
	struct stat st;
	stat(filename,&st);
	int filesize = st.st_size;
	int num_packets = filesize/payloadSize;
	if(num_packets%payloadSize!=0) num_packets++;
	printf("filesize: %d\n",filesize);
	int packets_completed = 0;
	uint32_t nsize =htonl((uint32_t)filesize);
	printf("Transfer will be completed in %d packets\n",num_packets);
	
	head = malloc(sizeof(struct window));
	head->seq_num = 0;
	head->payload = malloc(payloadSize);
	memset(head->payload,0,payloadSize);
	memcpy(head->payload,&nsize,sizeof(uint32_t));
	n = pthread_create(&(head->packet_thread),NULL,&packetLogic,head);
	if(n<0)syserr("unable to create threads");
	tail = head;
	int bytes_remaining = filesize;
	int bytes_read;
	struct timeval tv;
	fd_set set;
	pthread_t ackThread;
	n = pthread_create(&ackThread,NULL,&ackLogic,NULL);
	pthread_join(head->packet_thread,NULL);
	packets_completed++;
	while(bytes_remaining > 0){
			if(head->seq_num - tail->seq_num < windowSize){
				head->next = malloc(sizeof(struct window));
				head->next->seq_num = head->seq_num + 1;
				head = head->next;
				head->payload = malloc(payloadSize);
				if(bytes_remaining < payloadSize){
					bytes_read = fread(head->payload,bytes_remaining,1,f);
					if(bytes_read<0) syserr("can't read file\n");
					bytes_remaining-=(bytes_remaining*bytes_read);
				}
				else{
					bytes_read = fread(head->payload,payloadSize,1,f);
					if(bytes_read<0) syserr("can't read file\n");
					bytes_remaining-=(payloadSize*bytes_read);
				}
				
				n = pthread_create(&(head->packet_thread),NULL,&packetLogic,head);
				if(n<0)syserr("unable to create threads");
				//printf("%d bytes remaining to be sent\n",bytes_remaining);
			}
			if(tail->receivedACK){
				struct window *old = tail;
				tail = tail->next;
				pthread_join(old->packet_thread,NULL);
				free(old->payload);
				free(old);
				packets_completed++;
				if(packets_completed%100==0) printf("completed %d packets\n",packets_completed);
			}
	}
	printf("Transmission complete: %d packets sent.\n",packets_completed);
	time_t end_time = time(NULL);
	printf("Transmission completed in %d seconds\n",end_time-start_time);
	printf("Throughput = %dMB/s\n",(filesize/(end_time-start_time))/100000);
	fclose(f);
  close(sockfd);
  return 0;
}
