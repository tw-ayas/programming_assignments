#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include "shared.h"

int main(){
    key_t ShmKEY;
    int ShmID;

    ShmKEY = ftok(".", 'x');

    ShmID = shmget(ShmKEY, SHMSIZE, 0666);

    if (ShmID < 0) {;
        printf("*** shmget error (server) %s ***\n", strerror(errno));
        printf("*** Server has not started yet. ***\n");
        //exit(1);
    }
    printf("Client initialised Shared Memory Buffer to queue Requests...\n");

    Request *requestArray = (Request *)shmat(ShmID, NULL, 0);

    int endServer = 0;
    while(!endServer){
        while (1)
        {
            char bufC[MAXCOMMAND] = {0};
            char bufV[MAXCOMMAND] = {0};

            printf("Send Command: ");
            scanf("%s", bufC);
            int validCommand = 1;
            int command = -1;
            int value = 0;
            int posSpace = 0;

            if(strcmp(bufC, "") == 0){
                validCommand = 0;
            }
            if(strcmp(bufC, "INSERT") == 0 || strcmp(bufC, "insert") == 0){
                command = INSERT;
            }
            else if(strcmp(bufC, "READ") == 0 || strcmp(bufC, "read") == 0){
                command = READ;
            }
            else if(strcmp(bufC, "DELETE") == 0|| strcmp(bufC, "delete") == 0){
                command = DELETE;
            }
            else if(strcmp(bufC, "EXIT") == 0 || strcmp(bufC, "exit") == 0) {
                endServer = 1;
                break;
            }
            else{
                validCommand = 0;
            }

            if(validCommand){
                printf("Value:", bufC, command);
                scanf("%s", bufV);
                value = atoi(bufV);
            }

            if(validCommand && command >= INSERT && command <= DELETE) {
                int pos = -1;
                for(int i=0; i < QUEUELENGTH; i++){
                    while(atomic_flag_test_and_set(&(requestArray[i].locked)));
                    if(requestArray[i].status == FREE){
                        requestArray[i].status = QUEUED;
                        requestArray[i].command = command;
                        requestArray[i].value = value;
                        pos = i;
                        printf("\nExecute: %s %s\n\n", bufC, bufV);
                        atomic_flag_clear(&(requestArray[i].locked));
                        break;
                    }
                    atomic_flag_clear(&(requestArray[i].locked));
                }
                
                //pid_t pid_subshell = fork();
                //if(pid_subshell == -1){
                //    printf("Response executor failed...\n");
                //    exit(0);
                //}
                //else if(pid_subshell == 0){
                    while(1){
                        //sleep(1);
                        while(atomic_flag_test_and_set(&(requestArray[pos].locked)));
                        //printf("Waiting Req %d\n", pos);
                        if(requestArray[pos].status == AVAILABLE){
                            printf("\nServer Response:\n    %s\n\nSend Command:", requestArray[pos].buf);
                            requestArray[pos].status = FREE;
                            atomic_flag_clear(&(requestArray[pos].locked));
                            break;
                        }
                        atomic_flag_clear(&(requestArray[pos].locked));
                    }
                //    return 0;
                //}
            }
        }
    }

    shmctl(ShmID, IPC_RMID, NULL);
    printf("Removing Shared Memory Buffer...\n");
    printf("Client end.\n");

    return 0;
}
