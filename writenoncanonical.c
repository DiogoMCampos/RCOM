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
#include "auxiliar.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int count = 0;

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
	UA[4] != TRAMA_FLAG) 
    {
	return -1;
    }

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

    fd = open(argv[1], O_RDWR | O_NOCTTY);

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

    printf("New termios structure set\n");
    
    while (count < 3 && STOP == FALSE) {
        char* SET = (char*) malloc(5 * sizeof(char));
        createSET(SET);

        write(fd, SET, 5);

        sleep(1);

        char* UA = (char*) malloc(5 * sizeof(char));
        read(fd, UA, 5);
        
        if (verifyUA(UA) == 0) {
	    STOP = TRUE;
        }
        
        count++;
    }
    
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
    return 0;
}
