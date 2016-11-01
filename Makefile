CC = gcc
CFLAGS = -Wall

all: application

application: utilities.h datalink.h application.h application.c
	$(CC) application.c datalink.c -o application $(CFLAGS)

clean:
	rm -f application
