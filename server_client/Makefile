CC ?= cc 
CFLAGS ?= -g -Wall -O2

all: server client

server: server.c
	$(CC) $(CFLAGS) -o $@ $<

client: client.c
	$(CC) $(CFLAGS)  -g -o $@ $<
	
clean:
	rm -f server client