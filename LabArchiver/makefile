all: archiver

archiver: archiver.o
	gcc -o archiver archiver.o 

archiver.o: archiver.c
	gcc -c -W -Wall -O3 archiver.c

.PHONY: all clean

clean:
	rm -f *.o archiver