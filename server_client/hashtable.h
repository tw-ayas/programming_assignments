#define HASH_CONSTANT 129
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdatomic.h>
#include <string.h>

struct item{
    int value;
    struct item* next;
};
typedef struct item item;

struct bucket{
    item *start;
    int size;
    atomic_bool locked;
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
        atomic_flag_clear(&(table->buckets[i]->locked));
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

    while(atomic_flag_test_and_set(&(currentBucket->locked)));

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

    atomic_flag_clear(&(currentBucket->locked));

}

void delete(HashTable *table, int value){
    int index = hash_function(value, table->size);
    
    bucket *bucket = table->buckets[index];

    while(atomic_flag_test_and_set(&(bucket->locked)));

    item* current = bucket->start;
    item* n_item;
    if(current == NULL) {
        atomic_flag_clear(&(bucket->locked));
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
                    atomic_flag_clear(&(bucket->locked));
                    return;
                }
            }
            current->next = n_item->next;
            free_item(n_item);
        }
    }

    atomic_flag_clear(&(bucket->locked));
}

void readBucket(HashTable *table, int value, char *buf){
    int index = hash_function(value, table->size);

    bucket* currentBucket = table->buckets[index];
    char str[64];
    strcpy(str, "Bucket: ");
    while(atomic_flag_test_and_set(&(currentBucket->locked)));
    item *currentItem = currentBucket->start;
    while(currentItem != NULL ){
        char num[12];
        snprintf( num, sizeof(num), "%d -> ", currentItem->value);
        strcat(str, num);
        currentItem = currentItem->next;
    }
    atomic_flag_clear(&(currentBucket->locked));
    strcpy(buf, str);
}