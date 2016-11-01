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

	if (state == SENDER) {
		FILE* file = fopen(argv[3], "r+");
		if(file == NULL){
	  	printf("File doesn't exist.\n");
	  	return -1;
		}else{
			printf("Input file opened.\n");
		}
	}else{
		FILE* file = fopen(argv[3], "w+");
		if(file == NULL){
	  	printf("Failed to create file.\n");
	  	return -1;
		}else{
			printf("Output file created.\n");
		}
	}



	int fd = openSerial(argv[2], state);
	llopen(fd, state);
	llclose(fd,state);

	return 0;
}

int file_reader(FILE* file){
	return 0;
}

int sender(){

	return 0;
}

int receiver(){

	return 0;
}
