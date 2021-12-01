all: server client emergency_clean

server: server.o
	gcc -o server server.o -lm -pthread -lrt

server.o: server.c ids.h
	gcc -c -W -Wall -O3 server.c

client: client.o
	gcc -o client client.o -lm -pthread -lrt

client.o: client.c ids.h
	gcc -c -W -Wall -O3 client.c

emergency_clean: emergency_clean.o
	gcc -o emergency_clean emergency_clean.o -lm -pthread -lrt

emergency_clean.o: emergency_clean.c ids.h
	gcc -c -W -Wall -O3 emergency_clean.c

.PHONY: all clean

clean:
	rm -f *.o server client emergency_clean
