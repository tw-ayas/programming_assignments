#define QUEUED 0
#define EXECUTED 1
#define AVAILABLE 2
#define TAKEN 3
#define FREE 4
#define INSERT 5
#define READ 6
#define DELETE 7
#define KILL 8
#define MAXCOMMAND 64
#define RESPONSEBUF 256

#define QUEUELENGTH 32

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

struct Request{
    int status;
    int command;
    int value;
    char buf[RESPONSEBUF];
    atomic_bool locked;
};
typedef struct Request Request;

#define SHMSIZE sizeof(Request) * QUEUELENGTH
