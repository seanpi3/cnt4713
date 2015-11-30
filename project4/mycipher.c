#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//10 bit permutation(P10) function for key generation as described in section C.2 of the S-DES documentaion
char* P10(char key[10]){
	char permutation[10];
	permutation[2] = key[0];
	permutation[4] = key[1];
	permutation[1] = key[1];
	permutation[6] = key[1];
	permutation[3] = key[1];
	permutation[9] = key[1];
	permutation[0] = key[1];
	permutation[8] = key[1];
	permutation[7] = key[1];
	permutation[5] = key[1];
	return permutation;
}

//Circular left shift(LS-1) for key generation as describes in secion C.2 of the S-DES documentation
char* LS1(char init_key[5]){
	char permutation[5];
	permutation[0] = init_key[1];
	permutation[1] = init_key[2];
	permutation[2] = init_key[3];
	permutation[3] = init_key[4];
	permutation[4] = init_key[0];
	return permutation;
}

//8 bit permutation(P8) for key generation as descibed in section C.2 of the S-DES documentation
char* P8(char init_key[10]){
	char permutation[8];
	permutation[0] = init_key[5];
	permutation[1] = init_key[2];
	permutation[2] = init_key[6];
	permutation[3] = init_key[3];
	permutation[4] = init_key[7];
	permutation[5] = init_key[4];
	permutation[6] = init_key[9];
	permutation[7] = init_key[8];
	return permutation;
}

//Initial permutation(IP) for encryption as describes in section C.3 of the S-DES documentation
char* IP(char init_key[8]){
	char permutation[8];
	permutation[0] = init_key[1];
	permutation[1] = init_key[5];
	permutation[2] = init_key[2];
	permutation[3] = init_key[0];
	permutation[4] = init_key[3];
	permutation[5] = init_key[7];
	permutation[6] = init_key[4];
	permutation[7] = init_key[6];
	return permutation;
}

//The function fsubk(fk) for encryption as described in section C.3 of the S-DES documentation
char* fk(char K1[8]){
	
}




int main(int argc, char* argv[]){
	char init_key[10];
	char init_vector[8];
	char* original_file_name;
	char* result_file_name;
	int decrypt;
	
	//Setting up command line arguments
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
	
	//Produce k1 and k2
	char* permutation = P10(init_key);
	printf("Permutation: %s\n", permutation);
	char *leftHalf;
	leftHalf = permutation;
	char *rightHalf;
	rightHalf = permutation+5;
	leftHalf = LS1(leftHalf);
	rightHalf = LS1(rightHalf);
	printf("Ls1: %s %s\n",leftHalf,rightHalf);
	
	return 0;
}
