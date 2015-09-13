CC=g++
CFLAGS=-c -Wall

all: soma_demo

soma_demo: main.o lib.o soma.o libcsv.o 
	$(CC) lib.o soma.o main.o libcsv.o -lm -lmuparser -o soma_demo

main.o: main.c
	$(CC) $(CFLAGS) main.c

lib.o: lib.c
	$(CC) $(CFLAGS) lib.c

soma.o: soma.c
	$(CC) $(CFLAGS) soma.c

libcsv.o: libcsv.c
	$(CC) $(CFLAGS) libcsv.c

clean:
	rm -rf *o 
