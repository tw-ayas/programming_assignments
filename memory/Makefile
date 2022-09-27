CC ?= cc 
CFLAGS ?= -g -Wall -O2

all: memory.so mallocfree threadtest

memory.so : memory.c
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@  -ldl 

mallocfree: memory_tester.c
	$(CC) $(CFLAGS) -o $@ $<

threadtest: threadtest.c
	$(CC) $(CFLAGS)  -g -o $@ $<

check: all

clean:
	rm -f memory.so mallocfree threadtest