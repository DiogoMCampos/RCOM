CC = gcc
CFLAGS = -Wall

all: download

download: download.c download.h ftp.c ftp.h
	$(CC) download.c ftp.c -o download $(CFLAGS)

clean:
	rm -f download
