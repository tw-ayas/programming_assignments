#define HASH_CONSTANT 129
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdatomic.h>
#include <pthread.h>
#include <string.h>

struct item{
    int value;
    struct item* next;
};
typedef struct item item;

struct bucket{
    item *start;
    int size;
    int readerCount;
    pthread_mutex_t counter_lock, write_lock;
};
typedef struct bucket bucket;

struct HashTable{
    bucket** buckets;
    int size;
    int count;
};
typedef struct HashTable HashTable;

item* create_item(int value){
    item *i = (item *)malloc(sizeof(item));
    i->value = value;
    i->next = NULL;
    return i;
}

HashTable *create_hashtable(int size){
    HashTable* table = (HashTable *)malloc(sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->buckets = (bucket **)calloc(table->size, sizeof(bucket*));

    for(int i=0; i < table->size; i++){
        table->buckets[i] = malloc(sizeof(bucket));
        table->buckets[i]->start = NULL;
        table->buckets[i]->size = NULL;
        table->buckets[i]->readerCount = 0;
        pthread_mutex_unlock(&(table->buckets[i]->write_lock));
    }

    return table;
}

void free_item(item* item){
    free(item);
}

void free_table(HashTable* table){
    for(int i = 0; i < table->size; i++){
        bucket* bucket = table->buckets[i];
        item *item = bucket->start;
        while(item != NULL) {
            free_item(item);
            item = item->next;
        }
        free(bucket);
    }
    free(table);

}

int hash_function(value, size){
    return (value + HASH_CONSTANT) % size;
}

void insert(HashTable *table, int value){
    item *item, *current;
    bucket *currentBucket;
    item = create_item(value);

    int index = hash_function(value, table->size);
    currentBucket = table->buckets[index];

    pthread_mutex_lock(&(currentBucket->write_lock));

    current = currentBucket->start;
    if(current == NULL) {
        currentBucket->start = item;
        currentBucket->size++;
        table->count++;
    }
    else{
        while(current->next != NULL){
            current = current->next;
        }
        current->next = item;
    }

    pthread_mutex_unlock(&(currentBucket->write_lock));

}

void delete(HashTable *table, int value){
    int index = hash_function(value, table->size);
    
    bucket *bucket = table->buckets[index];

    pthread_mutex_lock(&(bucket->write_lock));

    item* current = bucket->start;
    item* n_item;
    if(current == NULL) {
        pthread_mutex_unlock(&(bucket->write_lock));
        return;
    }else {
        n_item = current->next;
    }
    if(current->value == value) {
        bucket->start = current->next;
        free_item(current);
    }
    else {
        if(n_item != NULL) {
            while (n_item->value != value) {
                current = current->next;
                n_item = current->next;
                if (n_item == NULL) {
                    atomic_flag_clear(&(bucket->write_lock));
                    return;
                }
            }
            current->next = n_item->next;
            free_item(n_item);
        }
    }

    pthread_mutex_unlock(&(bucket->write_lock));
}

void readBucket(HashTable *table, int value, char *buf){
    int index = hash_function(value, table->size);

    bucket* currentBucket = table->buckets[index];
    char str[64];
    strcpy(str, "Bucket: ");
    pthread_mutex_lock(&(currentBucket->counter_lock));
    currentBucket->readerCount++;
    if(currentBucket->readerCount == 1){
        pthread_mutex_lock(&(currentBucket->write_lock));
    }
    pthread_mutex_unlock(&(currentBucket->counter_lock));
    item *currentItem = currentBucket->start;
    while(currentItem != NULL ){
        char num[12];
        snprintf( num, sizeof(num), "%d -> ", currentItem->value);
        strcat(str, num);
        currentItem = currentItem->next;
    }
    pthread_mutex_lock(&(currentBucket->counter_lock));
    currentBucket->readerCount--;
    if(currentBucket->readerCount == 0){
        pthread_mutex_unlock(&(currentBucket->write_lock));
    }
    pthread_mutex_unlock(&(currentBucket->counter_lock));
    strcpy(buf, str);
}
