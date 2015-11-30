#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//10 bit permutation(P10) function for key generation as described in section C.2 of the S-DES documentaion
void P10(char* init_key){
	char permutation[10];
	permutation[0] = init_key[2];
	permutation[1] = init_key[4];
	permutation[2] = init_key[1];
	permutation[3] = init_key[6];
	permutation[4] = init_key[3];
	permutation[5] = init_key[9];
	permutation[6] = init_key[0];
	permutation[7] = init_key[8];
	permutation[8] = init_key[7];
	permutation[9] = init_key[5];
	int i;
	for(i=0;i<10;i++){
		init_key[i] = permutation[i];
	}
}

//Circular left shift(LS-1) for key generation as describes in secion C.2 of the S-DES documentation
void  LS1(char* init_key){
	char permutation[5];
	permutation[0] = init_key[1];
	permutation[1] = init_key[2];
	permutation[2] = init_key[3];
	permutation[3] = init_key[4];
	permutation[4] = init_key[0];
	int i;
	for(i=0;i<5;i++){
		init_key[i] = permutation[i];
	}
}

//8 bit permutation(P8) for key generation as descibed in section C.2 of the S-DES documentation
void P8(char* init_key){
	char permutation[8];
	permutation[0] = init_key[5];
	permutation[1] = init_key[2];
	permutation[2] = init_key[6];
	permutation[3] = init_key[3];
	permutation[4] = init_key[7];
	permutation[5] = init_key[4];
	permutation[6] = init_key[9];
	permutation[7] = init_key[8];
	int i;
	for(i=0;i<8;i++){
		init_key[i] = permutation[i];
	}
}

//Initial permutation(IP) for encryption as describes in section C.3 of the S-DES documentation
void IP(char* init_key){
	char permutation[8];
	permutation[0] = init_key[1];
	permutation[1] = init_key[5];
	permutation[2] = init_key[2];
	permutation[3] = init_key[0];
	permutation[4] = init_key[3];
	permutation[5] = init_key[7];
	permutation[6] = init_key[4];
	permutation[7] = init_key[6];
	int i;
	for(i=0;i<8;i++){
		init_key[i] = permutation[i];
	}
}

//The function fsubk(fk) for encryption as described in section C.3 of the S-DES documentation
char* fk(char K1[8]){
	
}




int main(int argc, char* argv[]){
	char* init_key;
	char* init_vector;
	char* original_file_name;
	char* result_file_name;
	int decrypt;
	
	//Setting up command line arguments
	if(argc > 6 || argc < 5){
		fprintf(stderr, "Usage: %s [-d] <init_key> <init_vector> <original_file> <result_file>\n",argv[0]);
		return 1; 
	}
	if(argc == 6){
		if(strcmp(argv[1],"[-d]") && strcmp(argv[1],"-d")){
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
		else init_key = argv[2];
		if(strlen(argv[3]) != 8){
			fprintf(stderr,"Init vector must be 8 bits.\n");
			return 1;
		}
		else init_vector = argv[3];
		original_file_name = argv[4];
		printf("The program will now decrypt the file %s with the key %s and the vector %s\n", original_file_name,init_key,init_vector);
	}
	else{
		decrypt = 0;
		if(strlen(argv[1]) != 10){
			fprintf(stderr,"Init key must be 10 bits.\n");
			return 1;
		}
		else init_key = argv[1];
		if(strlen(argv[2]) != 8){
			fprintf(stderr,"Init vector must be 8 bits.\n");
			return 1;
		}
		else init_vector = argv[2];
		original_file_name = argv[3];
		printf("The program will now encrypt the file %s with the key %s and the vector %s\n", original_file_name,init_key,init_vector);
	}
	
	//Produce k1 and k2
	char permutation[10];
	int i;
	for(i=0;i<10;i++){
		permutation[i] = init_key[i];
	}
	P10(permutation);
	printf("Permutation: %s\n", permutation);
	char leftHalf[5];
	char rightHalf[5];
	for(i=0;i<5;i++){
		leftHalf[i] = permutation[i];
		rightHalf[i] = permutation[i+5];
	}
	LS1(leftHalf);
	LS1(rightHalf);
	//printf("Ls1: %s %s\n",leftHalf,rightHalf);
	for(i=0;i<5;i++){
		permutation[i] = leftHalf[i];
		permutation[i+5] = rightHalf[i];
	}
	P8(permutation);
	char key1[8];
	for(i=0;i<8;i++) key1[i] = permutation[i];
	printf("Key1: %s\n",key1);
	for(i=0;i<2;i++){
		LS1(leftHalf);
		LS1(rightHalf);
	}	
	for(i=0;i<5;i++){
		permutation[i] = leftHalf[i];
		permutation[i+5] = rightHalf[i];
	}
	P8(permutation);
	char key2[8];
	for(i=0;i<8;i++) key2[i] = permutation[i];
	printf("Key2: %s\n",key2);
	
	return 0;
}
