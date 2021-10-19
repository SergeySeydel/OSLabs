all: mychmod

mychmod: mychmod.o
	gcc -o mychmod mychmod.o 

mychmod.o: mychmod.c
	gcc -c -W -Wall -O3 mychmod.c

.PHONY: all clean

clean:
	rm -f *.o mychmod
