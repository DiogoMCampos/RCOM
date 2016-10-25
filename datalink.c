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
