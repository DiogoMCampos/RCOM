#ifndef APPLICATION_H
#define APPLICATION_H

#include "datalink.h"
#include "utilities.h"

int sender(int fd, char* file);
int receiver(int fd);

int control_packet(char* packet, int type, char* name, int size);
long int unmount_control(char* packet, char* name);
#endif // APPLICATION_H
