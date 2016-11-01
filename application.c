#include "application.h"

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
	llopen(fd, state);
	llclose(fd,state);
/*
   char* trama1[255];
   char* data1[255];
   char* data2[255];

   createInfTrama(trama1, data1, 255, int ctrl_bit);
   int un = unmountTrama(char* TRAMA, char* data, int ctrl_bit);*/

	return 0;
}

int sender(){

	return 0;
}

int receiver(){

	return 0;
}
