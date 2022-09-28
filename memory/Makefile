CC ?= cc 
CFLAGS ?= -g -Wall -O2

all: memory.so mallocfree 

memory.so : memory.c
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@  -ldl 

mallocfree: memory_tester.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f memory.so mallocfree