#include "download.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: ./download [<user>:<password>@]<host>/<url-path>\n");
		exit(1);
	}

	char *url = malloc(URL_MAX * sizeof(char));
	strcpy(url, argv[1]);

    char* user = malloc(USER_MAX * sizeof(char));
    char* password = malloc(PASS_MAX * sizeof(char));
    char* host = malloc(HOST_MAX * sizeof(char));
    char* path = malloc(PATH_MAX * sizeof(char));

    parseUrl(url, host, user, password, path);
}

void parseUrl(char* url, char* host, char* user, char* password, char* path) {

}
