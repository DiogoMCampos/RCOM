#include "application.h"



int main(int argc, char** argv)
{
	int state;
	if(load_config() != 0){
		return -1;
	}

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

	if (llopen(fd, state) == FALSE) {
		printf("llopen failed!\n");
		return -1;
	}

	if (state == SENDER) {
		sender(fd, argv[3]);
	} else {
		receiver(fd);
	}

	if (llclose(fd, state) != 0) {
		return -1;
	}

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
	long int fl_size = ftell(s_file);
	fseek(s_file, 0, SEEK_SET);

	unsigned int totalLen = control_packet(start_packet, START, file, fl_size);
	llwrite(fd, start_packet, totalLen, 0);

	char *data = malloc(config.packet_size);
	char ctrl_bit = 0;
	char packetID = 0;
	long int written = 0;

	while (written < fl_size) {
		int read = fread(data, 1,  config.packet_size, s_file);
		int frame_size = read + 4;

		char *packet = malloc(sizeof(char) * frame_size);

		data_packet(data, packet, read, packetID);

		packetID++;
		packetID = packetID % 255;

		llwrite(fd, packet, frame_size, ctrl_bit);

		ctrl_bit ^= 1;
		written += read;
	}

	free(data);

	totalLen = control_packet(stop_packet, END, file, fl_size);
	llwrite(fd, stop_packet, totalLen, 0);

	return 0;
}

int receiver(int fd) {
	char* start_packet = malloc(255);

	llread(fd, start_packet, 0);

	char* name = malloc(sizeof(char) * 8);
	long int size = unmount_control(start_packet, name);
	printf("Size of file: %ld\n", size);
	printf("Name of file: %s\n", name);

	FILE* r_file = fopen(name, "w+");

	if (r_file == NULL) {
		printf("Failed to create file.\n");
		return -1;
	} else {
		printf("Output file created.\n");
	}

	int frame_size = config.packet_size + 4;
	char *frame = malloc(frame_size);
	char ctrl_bit = 0;
	char packetID = 0;
	long int read = 0;

	while (read < size) {
		int length = 0;

		while (length <= 0) {
			length = llread(fd, frame, ctrl_bit);
		}

		char* packet = malloc(config.packet_size);

		length = unmount_data(frame, packet, packetID);

		packetID++;
		packetID = packetID % 255;
		ctrl_bit ^= 1;
		read += length;

		fwrite(packet, sizeof(char), length, r_file);
	}

	free(frame);

	char* stop_packet = malloc(255);

	llread(fd, stop_packet, 0);

	name = malloc(sizeof(char) * 8);
	size = unmount_control(stop_packet, name);
	printf("Size of file: %ld\n", size);
	printf("Name of file: %s\n", name);

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

long int unmount_control(char* packet, char* name) {
	if (packet[0] != START && packet[0] != END) {
		return -1;
	}

	if ((int) packet[1] != 0) {
		return -1;
	}

	int sizeLen = packet[2];

	long int size = *((uint32_t*) &packet[3]);

	int j = 3 + sizeLen;

	if ((int) packet[j] != 1) {
		return -1;
	}

	int nameLen = packet[j + 1];

	unsigned int i;
	for (i = 0; i < nameLen; i++) {
		name[i] = packet[j + 2 + i];
	}

	return size;
}

void data_packet(char* buffer, char* dest, int size, unsigned int packetID) {
	unsigned char L2 = size / 256;
	unsigned char L1 = size % 256;

	dest[0] = DATA;
	dest[1] = packetID;
	dest[2] = L2;
	dest[3] = L1;
	memcpy(dest + 4, buffer, size);
}


int unmount_data(char *packet, char* dest, unsigned int packetID) {
	if (packet[0] != DATA ||
		packet[1] != packetID) {
			return -1;
		}

		unsigned char L1 = packet[2];
		unsigned char L2 = packet[3];

		int size = 256 * L1 + L2;
		memcpy(dest, packet + 4, size);

		return size;
	}

	int load_config() {
		FILE * conf = fopen(CONFIG_NAME, "r");


		int baudrate = 0;
		int packet = 0;
		int tOut = 0;
		int rMax = 0;

		if (conf == NULL) {
			printf("ERROR: Opening configuration file!\n");
			return -1;
		}else{
			printf("Configuration file opened\n");
		}

		char *line = malloc(256);
		size_t length;

		if(getline(&line, &length, conf) == -1){
			printf("Failed to read BAUDRATE\n");
			exit(1);
		}
		baudrate = strtol(line, NULL, 10);

		if(getline(&line, &length, conf) == -1){
			printf("Failed to read PACKET_SIZE\n");
			exit(1);
		}
		packet = strtol(line, NULL, 10);
		if(packet < 1){
			printf("PACKET_SIZE rejected, using PACKET_SIZE 128\n");
			packet = 128;
		}

		if(getline(&line, &length, conf) == -1){
			printf("Failed to read TIME_OUT\n");
			exit(1);
		}
		tOut = strtol(line, NULL, 10);
		if(tOut < 1){
			printf("TIME_OUT rejected, using TIME_OUT 3\n");
			tOut = 3;
		}

		if(getline(&line, &length, conf) == -1){
			printf("Failed to read RETRANS_MAX\n");
			exit(1);
		}
		rMax = strtol(line, NULL, 10);
		if(rMax < 1){
			printf("Rejected RETRANS_MAX, using  3\n");
			rMax = 3;
		}

		switch (baudrate) {
			case 9600:
				config.baudrate = B9600;
				break;
			case 19200:
				config.baudrate = B19200;
				break;
			case 38400:
				config.baudrate = B38400;
				break;
			case 57600:
				config.baudrate = B57600;
				break;
			case 115200:
				config.baudrate = B115200;
				break;
			default:
			config.baudrate = B38400;
				printf("Rejected baudrate using 38400!\n");
				break;
		}

		config.packet_size = packet;
		config.time_out = tOut;
		config.retrans_max = rMax;


		return 0;
	}
