#ifndef UTILITIES_H
#define UTILITIES_H

#define CONFIG_NAME "config.txt"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define MAX_SIZE 255
#define FALSE 0
#define TRUE 1
#define SENDER 0
#define RECEIVER 1
#define OPEN_SENDER O_RDWR | O_NOCTTY | O_NONBLOCK
#define OPEN_RECEIVER O_RDWR | O_NOCTTY

/* Flags */
#define TRAMA_FLAG 0x7E
#define A_SENDER 0x03
#define A_RECEIVER 0x01
#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0B
#define C_RR_0 0x05
#define C_RR_1 0x85
#define C_REJ_0 0x01
#define C_REJ_1 0x81
#define DATAPACKET 1
#define DATA 0x01
#define START 0x02
#define END 0x03
#define ESCAPE 0X7D
#define REPLACEMENT 0x20

/* State machine states */
#define START_STATE 0
#define FLAG_STATE 1
#define A_STATE 2
#define C_STATE 3
#define BCC_STATE 4
#define STOP_STATE 5

#endif // UTILITIES_H
