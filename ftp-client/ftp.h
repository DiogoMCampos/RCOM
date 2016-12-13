#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>

#define PORT 21
#define FTP_MSG_SIZE 1024

int connectSocket(const char* ip, int port);
int connectFtp(const char* ip);
void loginFtp(int sockfd, char* user, char* pass);
int passiveModeFtp(int sockfd);
void downloadFtp(char* path, char* file, int sockfd, int dataSocket);
void disconnectFtp(int sockfd);
int writeFtp(int sockfd, char* str, size_t size);
int readFtp(int sockfd, char* str, size_t size);
