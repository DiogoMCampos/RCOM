#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Max Length for URL parts
#define URL_MAX 256

// URL State Machine
#define USER_STATE 0
#define PASS_STATE 1
#define HOST_STATE 2
#define PATH_STATE 3

struct url {
	char user[URL_MAX];
	char pass[URL_MAX];
	char host[URL_MAX];
	char path[URL_MAX];
	char ip[URL_MAX];
};

void parseUrl(char* url, struct url* urlContents);
int getIP(struct url* url);
