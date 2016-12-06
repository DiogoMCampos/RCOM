#include "download.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: ./download [<user>:<password>@]<host>/<url-path>\n");
		exit(1);
	}

	char *url = malloc(URL_MAX * sizeof(char));
	strcpy(url, argv[1]);

    struct url* urlContents = malloc(sizeof(struct url));

    parseUrl(url, urlContents);

    free(url);
    free(urlContents);

    return 0;
}

void parseUrl(char* url, struct url* urlContents) {
    int state = USER_STATE;
    int i = 0;

    while (*url != '\0') {
        switch (state) {
            case USER_STATE:
                if (*url == ':') {
                    i = 0;
                    state = PASS_STATE;
                } else {
                    urlContents->user[i++] = *url;
                }
                ++url;
                break;
            case PASS_STATE:
                if (*url == '@') {
                    i = 0;
                    state = HOST_STATE;
                } else {
                    urlContents->pass[i++] = *url;
                }
                ++url;
                break;
            case HOST_STATE:
                if (*url == '/') {
                    i = 0;
                    state = PATH_STATE;
                } else {
                    urlContents->host[i++] = *url;
                }
                ++url;
                break;
            case PATH_STATE:
                urlContents->path[i++] = *url;
                ++url;
                break;
        }
    }
}
