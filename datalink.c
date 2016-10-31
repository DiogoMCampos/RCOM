#include "auxiliar.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define OPEN_SENDER O_RDWR | O_NOCTTY | O_NONBLOCK
#define OPEN_RECEIVER O_RDWR | O_NOCTTY

#define MAX_SIZE 4

#define TRIES 3
#define WAIT 3

#define START_STATE 0
#define FLAG_STATE 1
#define A_STATE 2
#define C_STATE 3
#define BCC_STATE 4
#define STOP_STATE 5

#define TRAMA_FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_UA 0x07

#define SENDER 0
#define RECEIVER 1

volatile int STOP = FALSE;

int alarm_flag = 1;
int count = 0;

struct linkLayer {
	char port[20];               /*Device /dev/ttySx, x = 0, 1*/
	int baudRate;                /* Transmission speed */
	unsigned int sequenceNumber; /*Sequence number: 0, 1*/
	unsigned int timeout;        /*Timer value: 1 s*/
	unsigned int numTransmissions; /*Number of trys if fails occurs*/
	char frame[MAX_SIZE];        /*Frame*/
};

void atende() {
	printf("Read count: %d\n", count);
	alarm_flag = 1;
	count++;
}

void setAlarm() {
	(void)signal(SIGALRM, atende);
	printf("Alarm set.\n");
}

void createSET(char *SET) {
	SET[0] = TRAMA_FLAG;
	SET[1] = A_SENDER;
	SET[2] = C_SET;
	SET[3] = SET[1] ^ SET[2];
	SET[4] = TRAMA_FLAG;
}

int verifySET(int fd, char *SET) {
	if (SET[0] != TRAMA_FLAG ||
	    SET[1] != A_SENDER ||
	    SET[2] != C_SET ||
	    SET[3] != (SET[1] ^ SET[2]) ||
	    SET[4] != TRAMA_FLAG) {
		return -1;
	}

	return 0;
}

void createUA(char *UA) {
	UA[0] = TRAMA_FLAG;
	UA[1] = A_SENDER;
	UA[2] = C_UA;
	UA[3] = UA[1] ^ UA[2];
	UA[4] = TRAMA_FLAG;
}

int verifyUA(char *UA) {
	if (UA[0] != TRAMA_FLAG ||
	    UA[1] != A_SENDER ||
	    UA[2] != C_UA ||
	    UA[3] != (UA[1] ^ UA[2]) ||
	    UA[4] != TRAMA_FLAG) {
		return -1;
	}

	return 0;
}

int llopen(int fd, int flag) {
	if (flag == RECEIVER) {
		int i = 0;
		int res;
		char buf[255];
		int counterTrama = 0; // Variable used to verify if the Trama was correctly read
		char temp;
		do {
			res = read(fd,&temp,1);   // returns after 5 chars have been input
			buf[i] = temp;

			if (buf[i] == TRAMA_FLAG)
				counterTrama++;
			i++;
		} while (counterTrama < 2);
		if (verifySET(fd, buf) == 0)
			printf("SET trama successfully received!\n");

		// Creating Trama
		char* UA = (char*) malloc(5 * sizeof(char));
		createUA(UA);

		res = write(fd, UA, 5);
		printf("%d bytes written\n", res);
		printf("UA successfully written!\n");

	}
	else {
		setAlarm();

		while (count < 3 && STOP == FALSE) {
			char* SET = (char*) malloc(5 * sizeof(char));
			createSET(SET);

			write(fd, SET, 5);
			sleep(1);

			if (alarm_flag) {
				printf("Alarm activated.\n");
				alarm(3);
				alarm_flag = 0;
			}

			char* UA = (char*) malloc(5 * sizeof(char));

			while (!alarm_flag) {
				read(fd, UA, 5);
			}

			if (verifyUA(UA) == 0) {
				printf("UA received correctly.\n");
				STOP = TRUE;
			}
		}
	}

	return 0;
}

int llwrite(int fd, char *buffer, int length) {

	/* if success
	   return numero de caracteres escritos */

	/* if error
	   return -1 */

	return 0;
}

int llread(int fd, char *buffer) {

	/* if success
	   return numero de caracteres lidos
	   if fails
	   return -1 */

	return 0;
}

int llclose(int fd) {
	return 0;
}

int machine_state = 0;
char charA;
char charC;

void state_machine(int state, char trama_char, int is_set) {

	switch (state) {
	case START_STATE:
		if (trama_char == TRAMA_FLAG)
			state = FLAG_STATE;
		break;

	case FLAG_STATE:
		if (trama_char == TRAMA_FLAG)
			state = FLAG_STATE;
		else if (trama_char == A_SENDER)
			state = A_STATE;
		else
			state = START_STATE;
		break;

	case A_STATE:
		if (trama_char == TRAMA_FLAG)
			state = FLAG_STATE;
		else if ((trama_char == C_SET && is_set) ||
		         (trama_char == C_UA && !is_set)) {
			state = C_STATE;
			charC = trama_char;
		} else
			state = START_STATE;
		break;

	case C_STATE:
		if (trama_char == TRAMA_FLAG)
			state = FLAG_STATE;
		else if (trama_char == (charA ^ charC))
			state = BCC_STATE;
		else
			state = START_STATE;
		break;

	case BCC_STATE:
		if (trama_char == TRAMA_FLAG)
			state = STOP_STATE;
		else
			state = START_STATE;
		break;

	default:
		break;
	}
}

int main(int argc, char *argv[]) {
	int fd, type;
	struct termios oldtio, newtio;

	if (argc < 3) {
		printf("Usage:\tnserial SENDER/RECEIVER SerialPort\n\tex: nserial SENDER "
		       "/dev/ttyS1\n");
		exit(1);
	} else if ((strcmp("/dev/ttyS0", argv[2]) != 0) &&
	           (strcmp("/dev/ttyS1", argv[2]) != 0)) {
		printf("Usage:\tnserial SENDER/RECEIVER SerialPort\n\tex: nserial SENDER "
		       "/dev/ttyS1\n");
		exit(2);
	}

	if (strcmp("SENDER", argv[1]) == 0) {
		type = SENDER;
	} else if (strcmp("RECEIVER", argv[1]) == 0) {
		type = RECEIVER;
	} else {
		printf("Usage:\tnserial SENDER/RECEIVER SerialPort\n\tex: nserial SENDER "
		       "/dev/ttyS1\n");
		exit(3);
	}

	int open_flags;

	if (type == SENDER) {
		open_flags = OPEN_SENDER;
	} else {
		open_flags = OPEN_RECEIVER;
	}

	fd = open(argv[2], open_flags);

	if (fd < 0) {
		perror(argv[1]);
		exit(-1);
	}

	if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 1; /* blocking read until 5 chars received */

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set.\n");

	llopen(fd, type);

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);
	return 0;
}
