#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct bit_map{
	unsigned int b1,b2,b3,b4,b5,b6,b7,b8,b9,b10;
};
struct bit_map init_key;
struct bit_map init_vector;

void set_init_key(char* key){
	init_key.b1 = key[0];
	init_key.b2 = key[1];
	init_key.b3 = key[2];
	init_key.b4 = key[3];
	init_key.b5 = key[4];
	init_key.b6 = key[5];
	init_key.b7 = key[6];
	init_key.b8 = key[7];
	init_key.b9 = key[8];
	init_key.b10 = key[9];
}
void set_init_vector(char* key){
	init_vector.b1 = key[0];
	init_vector.b2 = key[1];
	init_vector.b3 = key[2];
	init_vector.b4 = key[3];
	init_vector.b5 = key[4];
	init_vector.b6 = key[5];
	init_vector.b7 = key[6];
	init_vector.b8 = key[7];
}

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
		else set_init_key(argv[2]);
		if(strlen(argv[3]) != 8){
			fprintf(stderr,"Init vector must be 8 bits.\n");
			return 1;
		}
		else set_init_vector(argv[3]);
	}
	else{
		decrypt = 0;
		if(strlen(argv[1]) != 10){
			fprintf(stderr,"Init key must be 10 bits.\n");
			return 1;
		}
		else set_init_key(argv[1]);
		if(strlen(argv[2]) != 8){
			fprintf(stderr,"Init vector must be 8 bits.\n");
			return 1;
		}
		else set_init_vector(argv[2]);
	}
	
	return 0;
}
