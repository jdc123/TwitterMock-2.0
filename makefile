all: server

server: server.c
	gcc -o server -g -D_POSIX_C_SOURCE -std=c99 -Wall -pthread server.c -lpthread 

clean:
	rm -rf *o server
	
