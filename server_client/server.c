#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/shm.h>
#include <errno.h>
#include "hashtable.h"
#include <string.h>
#include "shared.h"

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Define Hashtable size...\n./server <hashtablesize>\n");
        exit(0);
    }

    int tablesize = atoi(argv[1]);
    HashTable *table = create_hashtable(tablesize);

    if (table < 0) {
        printf("Could not initialise Hashtable...\n");
        exit(0);
    }
    printf("Hashtable initialised with size %d...\n", tablesize);

    key_t ShmKEY;
    int ShmID;

    ShmKEY = ftok(".", 'x');
    ShmID = shmget(ShmKEY, SHMSIZE, IPC_CREAT | 0666);

    if (ShmID < 0) {
        printf("Removing existing Buffer. Reinitialising Buffer...\n");
        ShmID = shmget(ShmKEY, 0, 0);
        shmctl(ShmID, IPC_RMID, NULL);
        ShmID = shmget(ShmKEY, SHMSIZE, IPC_CREAT | 0666);
        if(ShmID < 0) {
            printf("*** shmget error (server) %s ***\n", strerror(errno));
            exit(1);
        }
    }
    printf("Server initialised Shared Memory Buffer to queue Requests...\n");
    Request *requestArray = (Request *)shmat(ShmID, NULL, 0);

    for(int i=0; i < QUEUELENGTH; i++){
        Request req;
        req.status = FREE;
        requestArray[i] = req;
    }

    int endServer = 0;
    while(!endServer){
        sleep(1);
        for(int i=0; i < QUEUELENGTH;i++){
            while(atomic_flag_test_and_set(&(requestArray[i].locked)));
            if(requestArray[i].status == QUEUED){
                //Process Request
                printf("requestPos:%d Command:%d Value:%d\n", i, requestArray[i].command, requestArray[i].value);
                switch (requestArray[i].command){
                    case INSERT:
                        insert(table, requestArray[i].value);
                        strcpy(requestArray[i].buf, "INSERT Completed");
                        break;
                    case READ:
                        readBucket(table, requestArray[i].value, requestArray[i].buf);
                        break;
                    case DELETE:
                        delete(table, requestArray[i].value);
                        strcpy(requestArray[i].buf, "DELETE Completed");
                        break;
                }
                requestArray[i].status = AVAILABLE;
            }
            atomic_flag_clear(&(requestArray[i].locked));
        }
    }

    shmctl(ShmID, IPC_RMID, NULL);
    printf("Removing Shared Memory Buffer...\n");
    printf("Server Exit!\n");
}