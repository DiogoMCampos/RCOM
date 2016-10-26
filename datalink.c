#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include "auxiliar.h"

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

int machine_state = 0;
char charA;
char charC;

void state_machine(int state, char trama_char, bool is_set){

  switch (state) {
    case START_STATE:
      if (strcmp(trama_char,TRAMA_FLAG))
        state = FLAG_STATE;
      break;

    case FLAG_STATE:
      if (strcmp(trama_char,TRAMA_FLAG))
        state = FLAG_STATE;
      else if (strcmp(trama_char,A_SENDER))
        state = A_STATE;
      else
        state = START_STATE;
      break;

    case A_STATE:
      if (strcmp(trama_char,TRAMA_FLAG))
        state = FLAG_STATE;
      else if ((strcmp(trama_char,C_SET) && is_set) ||
               (strcmp(trama_char,C_UA) && !is_set)) {
        state = C_STATE;
        charC = trama_char;
      }else
        state = START_STATE;
      break;

    case C_STATE:
      if (strcmp(trama_char,TRAMA_FLAG))
        state = FLAG_STATE;
      else if (strcmp(trama_char,charA^charC))
        state = BCC_STATE;
      else
        state = START_STATE;
      break;

    case BCC_STATE:
      if (strcmp(trama_char,TRAMA_FLAG))
        state = STOP_STATE;
      else
        state = START_STATE;
      break;

    default:
      break;
  }
}
