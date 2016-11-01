#include "application.h"

#define DATA 0x01
#define START 0x02
#define END 0x03

int main(int argc, char** argv)
{
	int state;

	if ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0)) {
		printf("Usage:\n\tex: application SENDER /dev/ttyS1 filename \n\tex: application RECEIVER /dev/ttyS1\n");
		exit(1);
	}

	if (argc == 4 && strcmp("SENDER", argv[1]) == 0) {
		state = SENDER;
	}

	else if (argc == 3 && strcmp("RECEIVER", argv[1]) == 0) {
		state = RECEIVER;
	}

	else {
		printf("Usage:\n\tex: application SENDER /dev/ttyS1 filename \n\tex: application RECEIVER /dev/ttyS1\n");
		exit(1);
	}

	char* asd = malloc(255);
	control_packet(asd,START,"pinguim.gif",1056);

/*
	int fd = openSerial(argv[2], state);
	llopen(fd, state);
*/
	if (state == SENDER) {
		//sender();
	}else{
		//receiver();
	}

/*	llclose(fd,state);*/

	return 0;
}

int sender(char* file){

	FILE* s_file = fopen(file, "r+");
	if(s_file == NULL){
		printf("File doesn't exist.\n");
		return -1;
	}else{
		printf("Input file opened.\n");
	}
	return 0;
}

int receiver(){
	FILE* r_file = fopen("asd.txt", "w+");
	if(r_file == NULL){
		printf("Failed to create file.\n");
		return -1;
	}else{
		printf("Output file created.\n");
	}

	return 0;
}

int getNrBytes(int x){
	int ret = 0;
	while (x != 0) {
		x >>= 8;
		ret++;
	}

	return ret;
}

void control_packet(char* packet, int type, char* name,int size){
	int i;

	int sizeLen = getNrBytes(size);
	int nameLen = strlen(name);
	int totalLen = sizeLen + nameLen + 5;

	packet = realloc(packet,totalLen);
	packet[0] = type;
	packet[1] = 0;
	packet[2] = sizeLen;

	for (i = 0; i < sizeLen; i++) {
		packet[3+i] = size >> (8*(sizeLen - 1 - i)) & 0xff;
	}

	int j = 3 + sizeLen;

	packet[j] = 1;
	packet[j+1] = nameLen;
	printf("%d\n", packet[j+1]);
	for (i = 0; i < nameLen; i++) {
		packet[j+2+i] = name[i];
	}
}
