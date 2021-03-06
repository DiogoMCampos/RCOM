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
	getIP(urlContents);

	int sockfd = connectFtp(urlContents->ip);
	loginFtp(sockfd, urlContents->user, urlContents->pass);
	int dataSocket = passiveModeFtp(sockfd);
	downloadFtp(urlContents->dir, urlContents->file, sockfd, dataSocket);
	disconnectFtp(sockfd);

	free(url);
	free(urlContents);

	return 0;
}

const char* regexLogin = "ftp://(([A-Za-z0-9])*:([A-Za-z0-9])*@)([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";
const char* regexAnon = "ftp://([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

void parseUrl(char* url, struct url* urlContents) {
	regex_t regexL, regexA;
	int urlLen = strlen(url);
	regmatch_t pmatch[urlLen];

	int mode;

	int retLogin = regcomp(&regexL, regexLogin, REG_EXTENDED);
	if (retLogin != 0) {
		perror("Failed to compiler regex");
		exit(1);
	}

	int resultLogin = regexec(&regexL, url, urlLen, pmatch, REG_EXTENDED);
	if (resultLogin != 0) {
		int retAnon = regcomp(&regexA, regexAnon, REG_EXTENDED);
		if (retAnon != 0) {
			perror("Failed to compiler regex");
			exit(1);
		}

		int resultAnon = regexec(&regexA, url, urlLen, pmatch, REG_EXTENDED);
		if (resultAnon != 0) {
			perror("Wrong url format");
			exit(1);
		} else {
			mode = ANON_MODE;
		}
	} else {
		mode = LOGIN_MODE;
	}

	url += 6; // ignore 'ftp://'

	if (mode == LOGIN_MODE) {
		parseUserLogin(&url, urlContents);
	} else {
		strncpy(urlContents->user, "anonymous", 9);
		strncpy(urlContents->pass, "pass", 4);
	}
	parseFtp(url, urlContents);
	parseFile(urlContents);
}

void parseUserLogin(char** url, struct url* urlContents) {
	int state = USER_STATE;
	int i = 0;

	while (state != HOST_STATE) {
		switch (state) {
		case USER_STATE:
			if (**url == ':') {
				i = 0;
				state = PASS_STATE;
			} else {
				urlContents->user[i++] = **url;
			}
			++(*url);
			break;
		case PASS_STATE:
			if (**url == '@') {
				i = 0;
				state = HOST_STATE;
			} else {
				urlContents->pass[i++] = **url;
			}
			++(*url);
			break;
		}
	}
}

void parseFtp(char* url, struct url* urlContents) {
	int state = HOST_STATE;
	int i = 0;

	while (*url != '\0') {
		switch (state) {
		case HOST_STATE:
			if (*url == '/') {
				i = 0;
				state = PATH_STATE;
			} else {
				urlContents->host[i++] = *url;
				++url;
			}
			break;
		case PATH_STATE:
			urlContents->path[i++] = *url;
			++url;
			break;
		}
	}
}

void parseFile(struct url* urlContents) {
	char* tmp = malloc(URL_MAX * sizeof(char));
	char* path = malloc(URL_MAX * sizeof(char));
	char* dir = malloc(URL_MAX * sizeof(char));

	strcpy(path, urlContents->path);

	int i = 0;

	while (*path != '\0') {
		tmp[i++] = *path;

		if (*path == '/') {
			strncat(dir, tmp, i);
			free(tmp);
			tmp = calloc(URL_MAX, sizeof(char));

			i = 0;
		}
		++path;
	}

	strcpy(urlContents->dir, dir);
	strcpy(urlContents->file, tmp);
}

int getIP(struct url* url) {
	struct hostent* h;

	if ((h = gethostbyname(url->host)) == NULL) {
		herror("gethostbyname");
		return 1;
	}

	char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));
	strcpy(url->ip, ip);

	return 0;
}
