C = gcc
CFLAGS = -Wall

all: writenoncanonical noncanonical

writenoncanonical: writenoncanonical.c 
	$(CC) writenoncanonical.c -o writenoncanonical $(CFLAGS)

noncanonical: noncanonical.c
	$(CC) noncanonical.c -o noncanonical $(CFLAGS)

clean:
	rm -f writenoncanonical noncanonical
