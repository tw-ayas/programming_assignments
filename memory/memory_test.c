#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(){
    char *ptr[10];
    ptr[0] = malloc(10);
    memset(ptr[0], 46, 10);
    *(ptr[0]) = 's';               // start
    *(ptr[0] + 10 - 1) = 'e'; // end
    fprintf(stdout, "%p %p %d %s\n\n", ptr[0], (ptr[0] + 10), 10, ptr[0]);
    fflush(stdout);
    return 0;
}