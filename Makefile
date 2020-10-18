keis = -pthread -Wall

all:
	gcc ${keis} string_array.c task.c -c 
	gcc ${keis} client.c string_array.o -o Client
	gcc ${keis} server.c task.o -D _GNU_SOURCE -o Server

clear:
	rm *.o Server Client
