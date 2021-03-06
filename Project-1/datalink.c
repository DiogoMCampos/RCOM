#include "datalink.h"

int alarmFlag = 1, count = 0;
volatile int STOP=FALSE;
int machine_state = 0;
char charA;
char charC;

int llopen(int fd, int flag) {
	initDataStats();

	if (flag == RECEIVER) {
		int i = 0;
		char buf[255];
		int counterTrama = 0;
		char temp;
		do {
			read(fd, &temp, 1);
			buf[i] = temp;

			if (buf[i] == TRAMA_FLAG) {
				counterTrama++;
			}

			i++;
		} while (counterTrama < 2);

		if (verifySET(buf) == 0) {
			STOP = TRUE;

			char* UA = (char*) malloc(5 * sizeof(char));
			createUA(UA);

			write(fd, UA, 5);
			free(UA);
		}
	}
	else {
		setAlarm();

		while (count < config.retrans_max && STOP == FALSE) {
			printf("llopen\n");
			char* SET = (char*) malloc(5 * sizeof(char));
			createSET(SET);
			write(fd, SET, 5);

			if (alarmFlag) {
				printf("Alarm activated.\n");
				alarm(config.time_out);
				alarmFlag = 0;
			}

			unsigned char* charC = malloc(sizeof(char));
			superviseStateMachine(fd, charC);
			free(SET);
		}

		alarm(0);
		if (STOP == TRUE) {
			printf("llopen successful!\n");
		}
	}

	return STOP;
}

int llwrite(int fd, char * buffer, int length, char ctrl_bit) {
	char* TRAMA = malloc(sizeof(char) * 2 * length + 6);
	unsigned int tramaLength = createInfTrama(TRAMA, buffer, length, ctrl_bit);

	count = 0;
	alarmFlag = 1;
	STOP = FALSE;

	while (count < config.retrans_max && STOP == FALSE) {
		write(fd, TRAMA, tramaLength);
		dataLinkStats.sent++;

		if (alarmFlag) {
			printf("Alarm activated.\n");
			alarm(config.time_out);
			alarmFlag = 0;
		}

		unsigned char* charC = malloc(sizeof(char));
		superviseStateMachine(fd, charC);

		if (!alarmFlag && !(*charC == C_RR_0 && ctrl_bit == 0) && 
		    !(*charC == C_RR_1 && ctrl_bit == 1)) {
			STOP = FALSE;
			dataLinkStats.resent++;
			dataLinkStats.rej++;
			count++;
		}

		free(charC);
	}

	alarm(0);
	free(TRAMA);

	return STOP;
}

int llread(int fd, char * buffer, char ctrl_bit, int artificial7e) {
	unsigned int i = 0;
	char* TRAMA = malloc(sizeof(char) * 2 * 256 + 6);
	int flagCounter = 0;

	if (artificial7e == TRUE) {	
		TRAMA[i] = 0x7e;
i++;
flagCounter++;
	}

	do {
		read(fd, &TRAMA[i], 1);
		if (TRAMA[i] == TRAMA_FLAG)
			flagCounter++;
			i++;
	} while (flagCounter < 2);

	unsigned int tramaLength = i;

	if (tramaLength < 6) {
		return -1;
	}

	char* destuffedData = malloc(sizeof(char) * tramaLength);
	int destuffedLength = unmountTrama(TRAMA, destuffedData, tramaLength, ctrl_bit);
	char* response = malloc(sizeof(char) * 5);
	if (destuffedLength > -1) {
		createRR(response, ctrl_bit);
		printf("RR sent\n");
		memcpy(buffer, destuffedData, destuffedLength);
	} else {
		createREJ(response, ctrl_bit);
		dataLinkStats.rej++;
		printf("REJ sent\n");

		if (destuffedLength == -2) {
			return -2;
		}
	}

	dataLinkStats.sent++;

	write(fd, response, 5);

	free(destuffedData);

	return destuffedLength;
}

int llclose(int fd, int flag) {
	if (flag == RECEIVER) {
		STOP = FALSE;
		alarmFlag = 0;

		unsigned char* charC = malloc(sizeof(char));
		superviseStateMachine(fd, charC);

		if (STOP == TRUE) {
			printf("DISC trama successfully received!\n");
		}

		STOP = FALSE;
		count = 0;
		alarmFlag = 1;

		while (count < config.retrans_max && STOP == FALSE) {
			char* DISC = (char*) malloc(5 * sizeof(char));
			createDISC(DISC);

			write(fd, DISC, 5);
			printf("DISC trama successfully written!\n");

			if (alarmFlag) {
				printf("Alarm activated.\n");
				alarm(config.time_out);
				alarmFlag = 0;
			}

			unsigned char* charC = malloc(sizeof(char));
			superviseStateMachine(fd, charC);
			free(DISC);
			free(charC);
		}
	}
	else {
		STOP = FALSE;
		count = 0;
		alarmFlag = 1;

		while (count < config.retrans_max && STOP == FALSE) {
			char* DISC = (char*) malloc(5 * sizeof(char));
			createDISC(DISC);

			write(fd, DISC, 5);
			printf("DISC trama successfully written!\n");

			if (alarmFlag) {
				printf("Alarm activated.\n");
				alarm(config.time_out);
				alarmFlag = 0;
			}

			unsigned char* charC = malloc(sizeof(char));
			superviseStateMachine(fd, charC);
			free(DISC);
			free(charC);
		}

		char* UA = (char*) malloc(5 * sizeof(char));
		createUA(UA);

		write(fd, UA, 5);
		free(UA);
		printf("UA trama successfully written!\n");
	}

	sleep(1);

	if (tcsetattr(fd, TCSANOW, &dataLink.oldtio) == -1) {
		fprintf(stderr, "%s\n", "Error closing link.");
		return 1;
	}

	if (STOP == FALSE) {
		printf("llclose failed!\n");
		return 1;
	} else {
		printf("llclose successful!\n");
	}

	close(fd);
	return 0;
}

int openSerial(char* port, int type){

	int fd;
	dataLink.port = port;

	if (type == SENDER)
		fd = open(dataLink.port, OPEN_SENDER);
	else
		fd = open(dataLink.port, OPEN_RECEIVER);

	if (fd < 0) {
		perror(dataLink.port);
		exit(-1);
	}

	if (tcgetattr(fd, &dataLink.oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&dataLink.newtio, sizeof(dataLink.newtio));
	dataLink.newtio.c_cflag = config.baudrate | CS8 | CLOCAL | CREAD;
	dataLink.newtio.c_iflag = IGNPAR;
	dataLink.newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	dataLink.newtio.c_lflag = 0;

	dataLink.newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	dataLink.newtio.c_cc[VMIN] = 1; /* blocking read until 5 chars received */

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &dataLink.newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set.\n");

	return fd;
}

void createSET(char* SET){
	SET[0] = TRAMA_FLAG;
	SET[1] = A_SENDER;
	SET[2] = C_SET;
	SET[3] = SET[1] ^ SET[2];
	SET[4] = TRAMA_FLAG;
}

void createUA(char* UA){
	UA[0] = TRAMA_FLAG;
	UA[1] = A_SENDER;
	UA[2] = C_UA;
	UA[3] = UA[1] ^ UA[2];
	UA[4] = TRAMA_FLAG;
}

void createDISC(char* DISC){
	DISC[0] = TRAMA_FLAG;
	DISC[1] = A_SENDER;
	DISC[2] = C_DISC;
	DISC[3] = DISC[1] ^ DISC[2];
	DISC[4] = TRAMA_FLAG;
}

unsigned int createInfTrama(char* TRAMA, char* data, int length, int ctrl_bit) {
	TRAMA[0] = TRAMA_FLAG;
	TRAMA[1] = A_SENDER;
	TRAMA[2] = (ctrl_bit == 1) ? 0b01000000 : 0b00000000;
	TRAMA[3] = TRAMA[1] ^ TRAMA[2];

	char bcc2 = createBCC2(data, length);

	char* destuffedData = malloc(sizeof(char) * length + 1);
	memcpy(destuffedData, data, length);
	destuffedData[length] = bcc2;

	char* stuffedData = malloc(sizeof(char) * length * 2);
	unsigned int stuffedLength = byteStuffing(destuffedData, stuffedData, length + 1);

	unsigned int i = 0;
	for (i = 0; i < stuffedLength; i++) {
		TRAMA[4 + i] = stuffedData[i];
	}

	TRAMA[4 + stuffedLength] = TRAMA_FLAG;

	free(destuffedData);
	free(stuffedData);

	return stuffedLength + 5;
}

int unmountTrama(char* TRAMA, char* destuffedData, int trama_length, int ctrl_bit) {
	if (ctrl_bit == 1) {
		if (TRAMA[0] != TRAMA_FLAG ||
		    TRAMA[1] != A_SENDER ||
		    TRAMA[2] != 0b01000000||
		    TRAMA[3] != (TRAMA[1] ^ TRAMA[2]) ||
		    TRAMA[trama_length - 1] != TRAMA_FLAG) {
			return -1;
		}
	} else if (TRAMA[0] != TRAMA_FLAG ||
	           TRAMA[1] != A_SENDER ||
	           TRAMA[2] != 0b00000000 ||
	           TRAMA[3] != (TRAMA[1] ^ TRAMA[2]) ||
	           TRAMA[trama_length - 1] != TRAMA_FLAG) {
		return -1;
	}

	unsigned char stuffedLength = trama_length - 5;
	char* stuffedData = malloc(sizeof(char) * stuffedLength);

	unsigned int i = 0;
	for (i = 0; i < stuffedLength; i++) {
		stuffedData[i] = TRAMA[4 + i];
	}

	unsigned int destuffedLength = byteDestuffing(stuffedData, destuffedData, stuffedLength);
	char* data = malloc(sizeof(char) * destuffedLength);
	memcpy(data, destuffedData, --destuffedLength);
	char bcc2 = createBCC2(data, destuffedLength);

	if (destuffedData[destuffedLength] != bcc2) {
		return -2;
	}

	free(data);

	return destuffedLength;
}

void createRR(char* RR, int packet) {

	RR[0] = TRAMA_FLAG;
	RR[1] = A_SENDER;

	if(packet == 0)
		RR[2] = C_RR_0;

	else if(packet == 1)
		RR[2] = C_RR_1;

	RR[3] = RR[1] ^ RR[2];
	RR[4] = TRAMA_FLAG;
}

void createREJ(char* REJ, int packet) {
	REJ[0] = TRAMA_FLAG;
	REJ[1] = A_SENDER;

	if(packet == 0)
		REJ[2] = C_REJ_0;

	else if(packet == 1)
		REJ[2] = C_REJ_1;

	REJ[3] = REJ[1] ^ REJ[2];
	REJ[4] = TRAMA_FLAG;
}

int verifySET(char * SET) {
	if (SET[0] != TRAMA_FLAG ||
	    SET[1] != A_SENDER ||
	    SET[2] != C_SET ||
	    SET[3] != (SET[1] ^ SET[2]) ||
	    SET[4] != TRAMA_FLAG)
		return -1;

	return 0;
}

int verifyUA(char* UA) {
	if (UA[0] != TRAMA_FLAG ||
	    UA[1] != A_SENDER ||
	    UA[2] != C_UA ||
	    UA[3] != (UA[1] ^ UA[2]) ||
	    UA[4] != TRAMA_FLAG) {
		return -1;
	}

	return 0;
}

int verifyDISC(char * DISC) {
	if (DISC[0] != TRAMA_FLAG ||
	    DISC[1] != A_SENDER ||
	    DISC[2] != C_DISC ||
	    DISC[3] != (DISC[1] ^ DISC[2]) ||
	    DISC[4] != TRAMA_FLAG)
		return -1;

	return 0;
}

void handleAlarm() {
	printf("Read count: %d\n", count);
	alarmFlag= 1;
	dataLinkStats.resent++;
	dataLinkStats.timeOuts++;
	count++;
}

void setAlarm() {
	(void) signal(SIGALRM, handleAlarm);
	printf("Alarm set.\n");
}

int byteStuffing(char* packet, char* dest, int size) {
	unsigned int stuffedSize = 0;
	unsigned int i;
	for (i = 0; i < size; i++) {
		if (packet[i] == TRAMA_FLAG) {
			dest[stuffedSize++] = ESCAPE;
			dest[stuffedSize++] = TRAMA_FLAG ^ REPLACEMENT;
		} else if (packet[i] == ESCAPE) {
			dest[stuffedSize++] = ESCAPE;
			dest[stuffedSize++] = ESCAPE ^ REPLACEMENT;
		} else {
			dest[stuffedSize++] = packet[i];
		}
	}

	return stuffedSize;
}

int byteDestuffing(char* packet, char* dest, int size) {
	unsigned int destuffedSize = 0;
	unsigned int i;
	for (i = 0; i < size; ++i) {
		if (i != (size - 1) && packet[i] == 0x7D) {
			++i;
			if (packet[i] == 0x5E) {
				dest[destuffedSize++] = 0x7E;
			} else if (packet[i] == 0x5D) {
				dest[destuffedSize++] = 0x7D;
			}
		} else {
			dest[destuffedSize++] = packet[i];
		}
	}
	return destuffedSize;
}

char createBCC2(char* buffer, int size) {
	int i;
	int bcc2 = 0;

	for (i = 0; i < size; i++) {
		bcc2 = bcc2 ^ buffer[i];
	}

	return bcc2;
}

int superviseStateMachine(int fd, unsigned char* C) {
	unsigned char tramaChar, charA, charC;
	int state = START_STATE;

	do {
		read(fd, &tramaChar, 1);
		switch (state) {
		case START_STATE:
			if (tramaChar == TRAMA_FLAG)
				state = FLAG_STATE;
			break;
		case FLAG_STATE:
			if (tramaChar == TRAMA_FLAG)
				state = FLAG_STATE;
			else
				state = A_STATE;

			charA = tramaChar;
			break;
		case A_STATE:
			if (tramaChar == TRAMA_FLAG)
				state = FLAG_STATE;
			else
				state = C_STATE;

			charC = tramaChar;
			memcpy(C, &charC, 1);
			break;
		case C_STATE:
			if (tramaChar == TRAMA_FLAG)
				state = FLAG_STATE;
			else if (tramaChar == (charA ^ charC))
				state = BCC_STATE;
			else
				state = START_STATE;
			break;
		case BCC_STATE:
			if (tramaChar == TRAMA_FLAG) {
				state = STOP_STATE;
				STOP = TRUE;
			}
			else
				state = START_STATE;
			break;
		default:
			break;
		}
	} while (state != STOP_STATE && !alarmFlag);

	if (charC == C_UA) {
		printf("Received UA supervision\n");
	} else if (charC == C_SET) {
		printf("Received SET supervision\n");
	} else if (charC == C_DISC) {
		printf("Received DISC supervision\n");
	} else if (charC == C_RR_0 || charC == C_RR_1) {
		printf("Received RR supervision\n");
	} else if (charC == C_REJ_0 || charC == C_REJ_1) {
		printf("Received REJ supervision\n");
	}

	if (state != STOP_STATE) {
		return -1;
	}

	return 0;
}

void initDataStats(){
	dataLinkStats.rej = 0;
	dataLinkStats.timeOuts = 0;
	dataLinkStats.sent = 0;
  dataLinkStats.resent = 0;
}

void printDataStats(int type){
	if (type == SENDER){
		printf("Timeouts: %d\n", dataLinkStats.timeOuts);
		printf("Resent packets: %d\n", dataLinkStats.resent);
		printf("Sent packets: %d\n", dataLinkStats.sent);
	}

	else {
		printf("Received packets: %d\n", dataLinkStats.sent);
		printf("Rej: %d\n", dataLinkStats.rej);
	}
}
