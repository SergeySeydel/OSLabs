all: result

result: main.o
	gcc -o result main.o -lm

main.o: main.c
	gcc -c -W -Wall -O3 main.c

.PHONY: all clean

clean:
	rm -f *.o result
