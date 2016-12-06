#ifndef DATALINK_H
#define DATALINK_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include "utilities.h"

struct linkLayer {
	char* port; /*Device /dev/ttySx, x = 0, 1*/
	unsigned int numTransmissions; /*Number of trys if fails occurs*/
	char frame[MAX_SIZE]; /*Frame*/
	struct termios oldtio;
	struct termios newtio;
};

struct configuration{
  int baudrate;
  unsigned int packet_size;
  unsigned int time_out;
  unsigned int retrans_max;
};

struct linkStats{
	unsigned int rej;
	unsigned int timeOuts;
	unsigned int sent;
  unsigned int resent;
};

struct linkLayer dataLink;
struct configuration config;
struct linkStats dataLinkStats;

int llopen(int fd, int flag);
int llwrite(int fd, char * buffer, int length, char ctrl_bit);
int llread(int fd, char * buffer, char ctrl_bit, int artificial7e);
int llclose(int fd, int flag);
int openSerial(char* port, int type);
void createSET(char* SET);
void createUA(char* UA);
void createDISC(char* DISC);
unsigned int createInfTrama(char* TRAMA, char* data, int data_length, int ctrl_bit);
int unmountTrama(char* TRAMA, char* destuffedData, int trama_length, int ctrl_bit);
void createRR(char* RR, int packet);
void createREJ(char* REJ, int packet);
int verifySET(char * SET);
int verifyUA(char* UA);
int verifyDISC(char * DISC);
void handleAlarm();
void setAlarm();
int byteStuffing(char* packet, char* dest, int size);
int byteDestuffing(char* packet, char* dest, int size);
char createBCC2(char* buffer, int size);
void state_machine(int state, char trama_char, int is_set);
int superviseStateMachine(int fd, unsigned char* C);
void initDataStats();
void printDataStats();

#endif // DATALINK_H
