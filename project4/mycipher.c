#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
	char init_key[10];
	char init_vector[8];
	char* original_file_name;
	char* result_file_name;
	int decrypt;
	
	if(argc > 6 || argc < 5){
		fprintf(stderr, "Usage: %s [-d] <init_key> <init_vector> <original_file> <result_file>\n",argv[0]);
		return 1; 
	}
	if(argc == 6){
		if(strcmp(argv[1],"[-d]") || strcmp(argv[1],"-d")){
			fprintf(stderr,"Unrecognized command %s\n",argv[1]);
			return 1;
		}
		else{
			decrypt = 1;
		}
		if(strlen(argv[2]) != 10){
			fprintf(stderr,"Init key must be 10 bits.\n");
			return 1;
		}
		if(strlen(argv[3]) != 8){
			fprintf(stderr,"Init vector must be 8 bits.\n");
			return 1;
		}
	}
	else{
		if(strlen(argv[1]) != 10){
			fprintf(stderr,"Init key must be 10 bits.\n");
			return 1;
		}
		if(strlen(argv[2]) != 8){
			fprintf(stderr,"Init vector must be 8 bits.\n");
			return 1;
		}
	}
	
	return 0;
}
