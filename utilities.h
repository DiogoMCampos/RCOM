#ifndef UTILITIES_H
#define UTILITIES_H

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define MAX_SIZE 255
#define FALSE 0
#define TRUE 1
#define SENDER 0
#define RECEIVER 1

/* Flags */
#define TRAMA_FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0B
#define ESCAPE 0X7D


#endif // UTILITIES_H
