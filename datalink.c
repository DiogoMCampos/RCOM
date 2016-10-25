#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include "auxiliar.h"

struct linkLayer {
    char port[20]; /*Device /dev/ttySx, x = 0, 1*/
    int baudRate; /* Transmission speed */
    unsigned int sequenceNumber;   /*Sequence number: 0, 1*/
    unsigned int timeout; /*Timer value: 1 s*/
    unsigned int numTransmissions; /*Number of trys if fails occurs*/
    char frame[MAX_SIZE]; /*Frame*/
}

int llopen(int port, int flag){

    /* if fail connection
            return -1 */

    return 0;
}

int llwrite(int fd, char * buffer, int length){

    /* if success
        return numero de caracteres escritos */

    /* if error
        return -1 */

    return 0;
}

int llread(int fd, char * buffer){

    /* if success
        return numero de caracteres lidos
    if fails
        return -1 */

    return 0;
}

int llclose(int fd){

    return 0;
}

int state = 0;
char charA;
char charC;

void state_machine(char trama_char){

  if (strcmp(trama_char,TRAMA_FLAG)) {
    if(state == 4)
     state = 5;
   else
      state = 1;
  }else if (strcmp(trama_char,A_SENDER)) {
    if(state == 1){
     state = 2;
     charA = trama_char;
   }else
      state = 0;
  }else if (strcmp(trama_char,C_RCV)) {
    if(state == 2){
     state = 3;
     charC = trama_char;
   }else
      state = 0;
  }else if (strcmp(trama_char,(charA ^ charC))) {
    if(state == 3)
     state = 4;
   else
      state = 0;
  }else{
    state = 0;
  }
}
