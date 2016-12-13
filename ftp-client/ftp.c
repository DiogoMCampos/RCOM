#include "ftp.h"

int connectSocket(const char* ip, int port) {
	int sockfd;
	struct sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);       /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);          /*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		exit(-1);
	}

	/*connect to the server*/
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("connect()");
		exit(-1);
	}

	return sockfd;
}

int connectFtp(const char* ip) {
	int sockfd = connectSocket(ip, PORT);

	char response[FTP_MSG_SIZE];

	if (readFtp(sockfd, response, sizeof(response)) < 0) {
		exit(-1);
	}

	return sockfd;
}

void loginFtp(int sockfd, char* user, char* pass) {
	char message[FTP_MSG_SIZE];

	sprintf(message, "USER %s\n", user);
	if (writeFtp(sockfd, message, strlen(message)) < 0) {
		exit(-1);
	}

	memset(message, 0, sizeof(message));

	if (readFtp(sockfd, message, sizeof(message)) < 0) {
		exit(-1);
	} else if (message[0] != '3' || message[1] != '3' || message[2] != '1') {
		exit(-2);
	}

	memset(message, 0, sizeof(message));

	sprintf(message, "PASS %s\n", pass);

	if (writeFtp(sockfd, message, strlen(message)) < 0) {
		exit(-1);
	}

	if (readFtp(sockfd, message, sizeof(message)) < 0) {
		exit(-1);
	} else if (message[0] != '2' || message[1] != '3' || message[2] != '0') {
		exit(-2);
	}
}

int passiveModeFtp(int sockfd) {
	char message[FTP_MSG_SIZE];

	sprintf(message, "PASV\n");

	if (writeFtp(sockfd, message, strlen(message)) < 0) {
		exit(-1);
	}

	if (readFtp(sockfd, message, sizeof(message)) < 0) {
		exit(-1);
	}

	unsigned int ip1, ip2, ip3, ip4;
	int port1, port2;

	if(sscanf(message, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2) < 0) {
		perror("Entering passive move");
		exit(-2);
	}

	int port = port1 * 256 + port2;

	memset(message, 0, sizeof(message));

	if (sprintf(message, "%d.%d.%d.%d", ip1, ip2, ip3, ip4) < 0) {
		perror("Building ip address");
		exit(-2);
	}

	int dataSocket = connectSocket(message, port);

	if (dataSocket < 0) {
		perror("Connecting to new socket");
		exit(-2);
	}

	return dataSocket;
}

void downloadFtp(char* path, char* file, int sockfd, int dataSocket) {
	FILE* downloadedFile;

    if ((downloadedFile = fopen(file, "w")) < 0) {
        perror(file);
        exit(-3);
    }

    int readBytes;

    char message[FTP_MSG_SIZE];

	sprintf(message, "CWD %s\n", path);

	if (writeFtp(sockfd, message, strlen(message)) < 0) {
		exit(-1);
	}

	if (readFtp(sockfd, message, sizeof(message)) < 0) {
		exit(-1);
	} else if (message[0] != '2' || message[1] != '5' || message[2] != '0') {
		exit(-2);
	}

	memset(message, 0, sizeof(message));

	sprintf(message, "RETR %s\n", file);

	if (writeFtp(sockfd, message, strlen(message)) < 0) {
		exit(-1);
	}

	if (readFtp(sockfd, message, sizeof(message)) < 0) {
		exit(-1);
	} else if (message[0] != '1' || message[1] != '5' || message[2] != '0') {
		exit(-2);
	}

    char buf[FTP_MSG_SIZE];

    while ((readBytes = read(dataSocket, buf, sizeof(buf))) > 0) {
        if (readBytes < 0) {
            perror("Receiving data");
            exit(-3);
        }

        if (fwrite(buf, readBytes, 1, downloadedFile) == 0) {
            perror("Saving data");
            exit(-3);
        }
    }

    fclose(downloadedFile);
    close(dataSocket);
}

void disconnectFtp(int sockfd) {
	char message[FTP_MSG_SIZE];

	if (readFtp(sockfd, message, sizeof(message)) < 0) {
		exit(-1);
	} else if (message[0] != '2' || message[1] != '2' || message[2] != '6') {
		exit(-2);
	}

	memset(message, 0, sizeof(message));

	sprintf(message, "QUIT\n");

	if (writeFtp(sockfd, message, strlen(message)) < 0) {
		exit(-1);
	}

	if (readFtp(sockfd, message, sizeof(message)) < 0) {
		exit(-1);
	} else if (message[0] != '2' || message[1] != '2' || message[2] != '1') {
		exit(-2);
	}

	close(sockfd);
}

int writeFtp(int sockfd, char* str, size_t size) {
	int written;

	if ((written = write(sockfd, str, size)) <= 0) {
		perror("Writing to socket");
		return -1;
	}

	printf("> %s", str);

	return written;
}

int readFtp(int sockfd, char* str, size_t size) {
	memset(str, 0, size);

	int received = recv(sockfd, str, size, 0);
	if (received < 0) {
		perror("Reading from socket");
		return -1;
	}

	printf("< %s", str);

	return received;
}
