C = gcc
CFLAGS = -Wall

all: writenoncanonical noncanonical

writenoncanonical: writenoncanonical.c auxiliar.h 
	$(CC) writenoncanonical.c -o writenoncanonical $(CFLAGS)

noncanonical: noncanonical.c auxiliar.h
	$(CC) noncanonical.c -o noncanonical $(CFLAGS)

clean:
	rm -f writenoncanonical noncanonical
