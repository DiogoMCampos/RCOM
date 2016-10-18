/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include "auxiliar.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    printf("New termios structure set\n");

	// TODO - COLOCAR ESTA PARTE NUMA FUNCAO À PARTE

    int i = 0;
	int counterTrama = 0; // Variable used to verify if the Trama was correctly read
    char temp;

    do {
        res = read(fd,&temp,1);   // returns after 5 chars have been input
	buf[i] = temp;
	printf("%d\n", counterTrama);
	printf("%02X\n", buf[i]);
	if (buf[i] == TRAMA_FLAG)
		counterTrama++;
	i++;
    } while (counterTrama < 2);


	if (buf[0] == TRAMA_FLAG)
		printf("1 trama flag correct!\n");
if (buf[1] == A_SENDER)
		printf("A_SENDER correct!\n");
if (buf[2] == C_SET)
		printf("C_SET correct!\n");
if (buf[3] == A_SENDER^C_SET)
		printf("BCC correct!\n");
if (buf[4] == TRAMA_FLAG)
		printf("2 trama flag correct!\n");



	

    buf[i]=0;               // so we can printf...
    printf(":%s\n", buf);
    printf("Message successfully received!\n\n");


    //work in progress---------------------------

    int length = strlen(buf);

    res = write(fd, buf, length + 1);
    printf("%d bytes written\n", res);
    printf("Message successfully written!\n");

    sleep(2);
    //-------------------------------------------

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
