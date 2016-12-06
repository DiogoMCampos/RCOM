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

    parseUrl(url, user, password, host, path);

    return 0;
}

void parseUrl(char* url, char* user, char* password, char* host, char* path) {
    int state = USER_STATE;
    int i = 0;

    while (*url != '\0') {
        switch (state) {
            case USER_STATE:
                if (*url == ':') {
                    i = 0;
                    state = PASS_STATE;
                } else {
                    user[i++] = *url;
                }
                ++url;
                break;
            case PASS_STATE:
                if (*url == '@') {
                    i = 0;
                    state = HOST_STATE;
                } else {
                    password[i++] = *url;
                }
                ++url;
                break;
            case HOST_STATE:
                if (*url == '/') {
                    i = 0;
                    state = PATH_STATE;
                } else {
                    host[i++] = *url;
                }
                ++url;
                break;
            case PATH_STATE:
                path[i++] = *url;
                ++url;
                break;
        }
    }
}
