#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>

#define MAX_ALLOC_SIZE 4096
#define MIN_ALLOC_SIZE 32
int niterations = 100; // Default number of iterations.
int nthreads = 1;      // Default number of threads.

void *worker(){
    char *ptr[niterations];
    int size[niterations];
        
    time_t t;
    srand((unsigned)time(&t));

    for (int i = 0; i < niterations; i++) {
        size[i] = rand() % MAX_ALLOC_SIZE + MIN_ALLOC_SIZE;
        fprintf(stderr, "%d, %d\n", i, size[i]);
        fflush(stderr);
        ptr[i] = malloc(size[i]);

        if (ptr[i] == NULL) {
            fprintf(stdout, "Fatal: failed to allocate %u bytes.\n", size[i]);
            exit(1);
        }

        /* access the allocated memory */
        int letter = rand() % 128 + 65;
        memset(ptr[i], letter, size[i]);
        
        *(ptr[i]) = 's';               // start
        *(ptr[i] + size[i] - 1) = 'e'; // end
        fprintf(stdout, "%p %p %d %s\n\n", ptr[i], (ptr[i] + size[i]), size[i], ptr[i]);
        fflush(stdout);
    }
    fprintf(stdout, "Thread memory allocation finished.\n");
    fflush(stdout);

    for (int i = 0; i < niterations; i++) {
        fprintf(stdout, "%c %c\n", *ptr[i],*(ptr[i] + size[i] - 1));
        if (!((*ptr[i] == 's' && *(ptr[i] + size[i] - 1) == 'e'))) {
            fprintf(stdout, "Memory content different than the expected\n");
            exit(1);
        }
        free(ptr[i]);
    }
    fprintf(stdout, "Thread memory freed.\n");
    fflush(stdout);

    return NULL;
}

int main(int argc, char *argv[]){
    fprintf(stderr, "Memory Tester.\n");
    fflush(stderr);

    if (argc >= 2) {
        nthreads = atoi(argv[1]);
    }

    if (argc >= 3) {
        niterations = atoi(argv[2]);
    }

    pthread_t *threads; 
    threads = (pthread_t *)malloc(nthreads * sizeof(pthread_t));

    int i;
    for (i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], NULL, &worker, NULL);
    }

    for (i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;

}

