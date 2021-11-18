all: forke

forke: forke.o
	gcc -o forke forke.o 

forke.o: forke.c
	gcc -c -W -Wall -O3 forke.c

.PHONY: all clean

clean:
	rm -f *.o forke