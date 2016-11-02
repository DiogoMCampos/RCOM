#ifndef APPLICATION_H
#define APPLICATION_H

#include "datalink.h"
#include "utilities.h"

#define DATA 0x01
#define START 0x02
#define END 0x03

#define PACKET_SIZE 512

int sender(int fd, char* file);
int receiver(int fd);

int control_packet(char* packet, int type, char* name, int size);
long int unmount_control(char* packet, char* name);
void data_packet(char* buffer, char* dest, int size, unsigned int packetID);
int unmount_data(char *packet, char* dest, unsigned int packetID);
#endif // APPLICATION_H
