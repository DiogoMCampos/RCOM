#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Max Length for URL parts
#define URL_MAX 256
#define USER_MAX 32
#define PASS_MAX 32
#define HOST_MAX 64
#define PATH_MAX 128

// URL State Machine
#define USER_STATE 0
#define PASS_STATE 1
#define HOST_STATE 2
#define PATH_STATE 3

void parseUrl(char* url, char* user, char* password, char* host, char* path);
