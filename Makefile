keis = -pthread -Wall

all:
	gcc ${keis} client-server.c -c 
	gcc ${keis} client.c client-server.o -o Client
	gcc ${keis} server.c client-server.o -o Server

clear:
	rm *.o Server Client
