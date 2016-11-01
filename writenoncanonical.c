/*Non-Canonical Input Processing*/

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
#include "auxiliar.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define TRIES 3
#define WAIT 3

volatile int STOP = FALSE;

int flag = 1;
int count = 0;

void atende() {
    printf("Read count: %d\n", count);
    flag = 1;
    count++;
}

void setAlarm() {
    (void) signal(SIGALRM, atende);
    printf("Alarm set.\n");
}

void createDISC(char* UA)
{
    UA[0] = TRAMA_FLAG;
    UA[1] = A_SENDER;
    UA[2] = C_DISC;
    UA[3] = UA[1] ^ UA[2];
    UA[4] = TRAMA_FLAG;
}

void createSET(char* SET)
{
    SET[0] = TRAMA_FLAG;
    SET[1] = A_SENDER;
    SET[2] = C_SET;
    SET[3] = SET[1] ^ SET[2];
    SET[4] = TRAMA_FLAG;
}

int verifyUA(char* UA)
{
    if (UA[0] != TRAMA_FLAG ||
	    UA[1] != A_SENDER ||
	    UA[2] != C_UA ||
	    UA[3] != (UA[1] ^ UA[2]) ||
	    UA[4] != TRAMA_FLAG) {
	       return -1;
    }

    return 0;
}

int verifyDISC(int fd, char * SET){

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

    if (SET[0] != TRAMA_FLAG ||
    SET[1] != A_SENDER ||
    SET[2] != C_DISC ||
    SET[3] != (SET[1] ^ SET[2]) ||
    SET[4] != TRAMA_FLAG)
    return -1;

    return 0;
}

int main (int argc, char** argv)
{
    int fd;
    struct termios oldtio, newtio;

    if ((argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0))) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);

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

    newtio.c_cc[VTIME] = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN] = 5;   /* blocking read until 5 chars received */

  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set.\n");

    setAlarm();

    while (count < 3 && STOP == FALSE) {
        char* SET = (char*) malloc(5 * sizeof(char));
        createSET(SET);

        write(fd, SET, 5);
        sleep(1);

        if (flag) {
            printf("Alarm activated.\n");
            alarm(3);
            flag = 0;
        }

        char* UA = (char*) malloc(5 * sizeof(char));

        while (!flag) {
            read(fd, UA, 5);
        }

        if (verifyUA(UA) == 0) {
            printf("UA received correctly.\n");
            STOP = TRUE;
        }
    }

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
    return 0;
}
