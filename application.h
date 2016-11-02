#ifndef APPLICATION_H
#define APPLICATION_H

#include "datalink.h"
#include "utilities.h"

int sender(char* file);
int receiver();

int getNrBytes(int x);
void control_packet(char* packet, int type, char* name,int size);
int unmount_control(char* packet, char* name);
#endif // APPLICATION_H
