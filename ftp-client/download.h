#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define URL_MAX 256
#define USER_MAX 32
#define PASS_MAX 32
#define HOST_MAX 64
#define PATH_MAX 128

void parseUrl(char* url, char* host, char* user, char* password, char* path);
