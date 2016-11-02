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


	int fd = openSerial(argv[2], state);
	//llopen(fd, state);

	if (state == SENDER) {
		sender(fd, argv[3]);
	} else {
		receiver(fd);
	}

	llclose(fd,state);

	return 0;
}

int sender(int fd, char* file) {
	FILE* s_file = fopen(file, "r+");

	if(s_file == NULL) {
		printf("File doesn't exist.\n");
		return -1;
	} else {
		printf("Input file opened.\n");
	}

	char* start_packet = malloc(255);
	char* stop_packet = malloc(255);

	fseek(s_file, 0, SEEK_END);
	int fl_size = ftell(s_file);
	fseek(s_file, 0, SEEK_SET);

	unsigned int totalLen = control_packet(start_packet, START, file, fl_size);

	// TESTING
	char* tempName = malloc(1);
	int x = unmount_control(start_packet, tempName);
	printf("Size of file: %d\n", x);
	printf("Name of file: %s\n", tempName);
	// TESTING

	llwrite(fd, start_packet, totalLen, 0);
	printf("%s\n", start_packet);

	control_packet(stop_packet, END, file, fl_size);

	return 0;
}

int receiver(int fd) {
	char* start_packet = malloc(255);

	llread(fd, start_packet, 0);
	printf("%02x\n", start_packet[0]);
	printf("%s\n", start_packet);
	// TESTING
	char* tempName = malloc(1);
	int x = unmount_control(start_packet, tempName);
	printf("Size of file: %d\n", x);
	printf("Name of file: %s\n", tempName);
	// TESTING

	FILE* r_file = fopen("asd.txt", "w+");
	if (r_file == NULL) {
		printf("Failed to create file.\n");
		return -1;
	} else {
		printf("Output file created.\n");
	}

	return 0;
}

int control_packet(char* packet, int type, char* name, int size) {
	int i;

	int sizeLen = 4;
	int nameLen = strlen(name);
	int totalLen = sizeLen + nameLen + 5;

	packet = realloc(packet, totalLen);
	packet[0] = type;
	packet[1] = 0;
	packet[2] = sizeLen;

	for (i = 0; i < sizeLen; i++) {
		packet[3 + i] = (size >> 8 * i) & 0xff;
	}

	int j = 3 + sizeLen;

	packet[j] = 1;
	packet[j + 1] = nameLen;

	for (i = 0; i < nameLen; i++) {
		packet[j + 2 + i] = name[i];
	}

	return totalLen;
}

int data_packet(char* packet, int size, unsigned char packetID){
	char* temp = malloc(size);
	memcpy(temp, packet, size);
	unsigned char l2 = (unsigned char)(size / 256);
	unsigned char l1 = (unsigned char)(size % 256);
	int sizeTemp = size+4;

	packet = realloc(packet, sizeTemp);
	packet[0] = DATAPACKET;
	packet[1] = packetID;
	packet[2] = l2;
	packet[3] = l1;

	memcpy(packet+4, temp, size);
	free(temp);

	return sizeTemp;
}

long int unmount_control(char* packet, char* name){
	printf("%02x\n", packet[0]);
	if (packet[0] != START && packet[0] != END) {
		return -1;
	}

	if ((int) packet[1] != 0) {
		printf("2\n");
		return -1;
	}

	int sizeLen = packet[2];

	long int size = *((uint32_t*) &packet[3]);

	int j = 3 + sizeLen;

	if ((int) packet[j] != 1) {
		printf("3\n");
		return -1;
	}

	int nameLen = packet[j + 1];

	unsigned int i;
	for (i = 0; i < nameLen; i++) {
		name[i] = packet[j + 2 + i];
	}

	return size;
}
