#ifndef DATALINK_H
#define DATALINK_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include "utilities.h"

#define OPEN_SENDER O_RDWR | O_NOCTTY | O_NONBLOCK
#define OPEN_RECEIVER O_RDWR | O_NOCTTY

struct linkLayer {
	char* port; /*Device /dev/ttySx, x = 0, 1*/
	unsigned int numTransmissions; /*Number of trys if fails occurs*/
	char frame[MAX_SIZE]; /*Frame*/
	struct termios oldtio;
	struct termios newtio;
};


int llopen(int fd, int flag);
int llwrite(int fd, char * buffer, int length);
int llread(int fd, char * buffer);

// TODO - USAR FLAG AQUI PARA SABER SE É O SENDER OU RECEIVER?
int llclose(int fd, int flag);
int openSerial(char* port, int type);
void createSET(char* SET);
void createUA(char* UA);
void createDISC(char* DISC);
void createInfTrama(char* TRAMA, char* data, int data_length, int ctrl_bit);
int unmountTrama(char* TRAMA, char* data, int trama_length, int ctrl_bit);
void createRR(char* RR, int packet);
void createREJ(char* REJ, int packet);
int verifySET(char * SET);
int verifyUA(char* UA);
int verifyDISC(char * DISC);
void handleAlarm();
void setAlarm();
// TODO - FALTA IMPLEMENTAR
int byteStuffing(char* packet, char* dest, int size);
// TODO - FALTA IMPLEMENTAR
int byteDestuffing(char* packet, char* dest, int size);
char createBCC(char* buffer, int size);
void state_machine(int state, char trama_char, int is_set);


#endif // DATALINK_H
