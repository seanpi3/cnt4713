#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Converts a byte string from ASCII to binary
void to_binary(char* key, int bytes){
	char* temp = malloc(bytes);
	int i;
	for(i=0;i<bytes;i++){
		if(key[i]=='1'){
			temp[i] = 0x01;
		}
		else{
			temp[i] = 0x00;
		}
	}
	for(i=0;i<bytes;i++){
		key[i] = temp[i];
	}
	//free(temp);
}

//Converts a byte to an array of bits
void to_byte_array(char byte, char *array, int bytes){
		int i;
		for(i=0;i<bytes;i++){
			array[i] = (byte >> ((bytes-1) - i)) & 0x01;
			//printf("%x\n",array[i]);
		}
}

//Convets an array of bits to a single byte
char to_byte(char *array,int bytes){
	int i;
	char out = 0x00;
	for(i=0;i<bytes;i++){
		out = out << 1;
		out = out | array[i];
	}
	return out;
}

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

//Initial permutation(IP) for encryption as described in section C.3 of the S-DES documentation
void IP(char* text_byte){
	char* permutation = malloc(8);
	permutation[0] = text_byte[1];
	permutation[1] = text_byte[5];
	permutation[2] = text_byte[2];
	permutation[3] = text_byte[0];
	permutation[4] = text_byte[3];
	permutation[5] = text_byte[7];
	permutation[6] = text_byte[4];
	permutation[7] = text_byte[6];
	int i;
	for(i=0;i<8;i++){
		text_byte[i] = permutation[i];
	}
}

//Inverse of the initial permutation function(IP) as described in section C.3 of the S-DES documentation
void IP_inv(char* bytes){
	char permutation[8];
	permutation[0] = bytes[3];
	permutation[1] = bytes[0];
	permutation[2] = bytes[2];
	permutation[3] = bytes[4];
	permutation[4] = bytes[6];
	permutation[5] = bytes[1];
	permutation[6] = bytes[7];
	permutation[7] = bytes[5];
	int i;
	for(i=0;i<8;i++){
		bytes[i] = permutation[i];
	}
}

//S0 Box for the fk function
void S0(char* p0, char* p3){
	char box[4][4];
	box[0][0] = 1;
	box[1][0] = 0;
	box[2][0] = 3;
	box[3][0] = 2;
	box[0][1] = 3;
	box[1][1] = 2;
	box[2][1] = 1;
	box[3][1] = 0;
	box[0][2] = 0;
	box[1][2] = 2;
	box[2][2] = 1;
	box[3][2] = 3;
	box[0][3] = 3;
	box[1][3] = 1;
	box[2][3] = 3;
	box[3][3] = 2;
	int row = (int)(p0[0] << 1) | p0[3];
	int column = (int)(p0[1] << 1) | p0[2];
	p3[0] = (box[column][row]>>1) & 0x01;
	p3[1] = box[column][row] & 0x01;
	//printf("%d,%d = %d\n",column,row,box[column][row]);
	//printf("%x%x\n",p3[0],p3[1]);
}

//S1 Box for the fk function
void S1(char* p1, char* p3){
	char box[4][4];
	box[0][0] = 0;
	box[1][0] = 1;
	box[2][0] = 2;
	box[3][0] = 3;
	box[0][1] = 2;
	box[1][1] = 0;
	box[2][1] = 1;
	box[3][1] = 3;
	box[0][2] = 3;
	box[1][2] = 0;
	box[2][2] = 1;
	box[3][2] = 0;
	box[0][3] = 2;
	box[1][3] = 1;
	box[2][3] = 0;
	box[3][3] = 3;
	int row = (int)(p1[0] << 1) | p1[3];
	int column = (int)(p1[1] << 1) | p1[2];
	p3[2] = (box[column][row]>>1) & 0x01;
	p3[3] = box[column][row] & 0x01;
	//printf("%d,%d = %d\n",column,row,box[column][row]);
	//printf("%x%x\n",p3[2],p3[3]);
}

//The function fsubk(fk) for encryption as described in section C.3 of the S-DES documentation
void fk(char* text_byte, char* K){
		char L,R;
		int i;
		char *right = malloc(4);
		/*
		L = text_byte & 0xF0;
		R = text_byte & 0x0F;
		for(i=0;i<4;i++){
			right[i] = (R >> (4-i)) & 0x01;
		}
		*/
		for(i=0;i<4;i++){
			right[i] = text_byte[i+4];
		}
		char *perm_exp = malloc(8);
		perm_exp[0] = right[3];
		perm_exp[1] = right[0];
		perm_exp[2] = right[1];
		perm_exp[3] = right[2];
		perm_exp[4] = right[1];
		perm_exp[5] = right[2];
		perm_exp[6] = right[3];
		perm_exp[7] = right[0];
		for(i=0;i<8;i++){
			//printf("%02x to ",perm_exp[i]);
			perm_exp[i] = perm_exp[i]^K[i];
			//printf("%02x\n",perm_exp[i]);
		}
		char p0[4];
		char p1[4];
		for(i=0;i<4;i++){
			p0[i] = perm_exp[i];
			p1[i] = perm_exp[4+i];
		}
		char* p3 = malloc(4);
		S0(p0,p3);
		S1(p1,p3);
		char* p4 = malloc(4);
		p4[0] = p3[1];
		p4[1] = p3[3];
		p4[2] = p3[2];
		p4[3] = p3[0];
		for(i=0;i<4;i++){
			text_byte[i] = text_byte[i] ^ p4[i];
		}
}

//Switch function(SW)
void SW(char* bytes){
	char* perm = malloc(8);
	int i;
	for(i=0;i<4;i++){
		perm[i] = bytes[4+i];
		perm[4+i] = bytes[i];
	}
	for(i=0;i<8;i++){
		bytes[i] = perm[i];
	}	
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
		result_file_name = argv[5];
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
		result_file_name = argv[4];
		printf("The program will now encrypt the file %s with the key %s and the vector %s\n", original_file_name,init_key,init_vector);
	}

	//Open file
	FILE *fin, *fout;
	fin = fopen(original_file_name, "rb");	
	fout = fopen(result_file_name,"wb");
	if(fin==NULL || fout==NULL) {
		printf("Error: cannot open files\n");
		return 0;
	}

		//Produce k1 and k2
		char* permutation = malloc(sizeof(char)*10);
		int i;
		for(i=0;i<10;i++){
			permutation[i] = init_key[i];
		}
		P10(permutation);
		//printf("Permutation: %s\n", permutation);
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
		char* key1 = malloc(sizeof(char)*8);
		for(i=0;i<8;i++) key1[i] = permutation[i];
		//printf("Key1: %s\n",key1);
		for(i=0;i<2;i++){
			LS1(leftHalf);
			LS1(rightHalf);
		}	
		for(i=0;i<5;i++){
			permutation[i] = leftHalf[i];
			permutation[i+5] = rightHalf[i];
		}
		P8(permutation);
		char* key2 = malloc(sizeof(char)*8);
		for(i=0;i<8;i++) key2[i] = permutation[i];
		//printf("Key2: %s\n",key2);
		to_binary(key1,8);
		to_binary(key2,8);
		int n;
		char *text_byte = malloc(1);
		char *write_byte = malloc(1);
	//Encryption
	if(!decrypt){
		while(n = fread(text_byte,1,1,fin)){
			char *bit_array = malloc(8);
			to_byte_array(*text_byte, bit_array,8);
			IP(bit_array);
			fk(bit_array,key1);
			SW(bit_array);
			fk(bit_array,key2);
			IP_inv(bit_array);
			*write_byte = to_byte(bit_array,8);	
			n = fwrite(write_byte,1,1,fout);
		}
	}
	//Decryption
	else{
		while(n = fread(text_byte,1,1,fin)){
			char *bit_array = malloc(8);
			to_byte_array(*text_byte, bit_array,8);
			IP(bit_array);
			fk(bit_array,key2);
			SW(bit_array);
			fk(bit_array,key1);
			IP_inv(bit_array);
			*write_byte = to_byte(bit_array,8);	
			n = fwrite(write_byte,1,1,fout);
		}
	}
	return 0;
}
