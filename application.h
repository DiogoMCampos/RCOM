#ifndef APPLICATION_H
#define APPLICATION_H

#include "datalink.h"
#include "utilities.h"

int sender(char* file);
int receiver(int fd);

int getNrBytes(int x);
void control_packet(char* packet, int type, char* name,int size);
int data_packet(char* packet, int size, unsigned char packetID);
#endif // APPLICATION_H
