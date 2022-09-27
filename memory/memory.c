#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

struct block_meta{
    size_t size;
    struct block_meta *next;
    struct block_meta *prev;
    int free;
};

typedef struct block_meta * block;

#define BLOCK_SIZE sizeof(struct block_meta)
#define align(x) ((((x-1) >> 3) << 3) + 8)

pthread_mutex_t extending_heap_lock = PTHREAD_MUTEX_INITIALIZER;

__thread block global_head = NULL; //Each thread gets its own head

void *extend_heap(block last, size_t size){
    block new_block;

    pthread_mutex_lock(&extending_heap_lock);
    new_block = sbrk(0);

    void *new_space = sbrk(size + BLOCK_SIZE);
    if(new_space == (void *)-1)
        return NULL;
    pthread_mutex_unlock(&extending_heap_lock);

    //Add new Block into List structure
    new_block->size = size;
    new_block->next = NULL;
    new_block->prev = last;

    if(last){
        last->next = new_block;
    }

    new_block->free = 0;

    return new_block;

}

void *find_best_fit_block(block *last, size_t size){
    //Finding the best block
    block best_block = NULL;
    block current_block = global_head;

    while(current_block != NULL){
        if(
            (current_block->free && (current_block->size >= size + BLOCK_SIZE)) && 
            (best_block == NULL || current_block->size < best_block->size + BLOCK_SIZE)
        ){
            //assign best_block when current_block size is less than best_block size || or when best_block is not initialised;
            best_block = current_block;
            if(best_block->size == size + BLOCK_SIZE)
                break;
        }
        current_block = current_block->next;
    }

    //after best_block is found
    //Loop the list and return the best_block || when best_block cannot be found return null
    current_block = global_head; 

    while(current_block != NULL){
        if((best_block - current_block) == 0){
            return current_block;
        }
        current_block = current_block->next;
        if(current_block)
            *last = current_block; //we keep storing the current block into last which can be used to extend heap if best_block is null.
    }

    return current_block;   
}

void split_block(block to_split, size_t size){
    block new;

    if(to_split){
        new = (block)((void *)to_split + size + BLOCK_SIZE);
        new->size = to_split->size - size - BLOCK_SIZE;
        new->next = to_split->next;
        new->prev = to_split;
        new->free = 1;
        to_split->next = new;
        to_split->size = size;

        if(new->next){
            new->next->prev = new;
        }
    }
}

void *malloc(size_t size){
    //malloc implementation without locks using Thread-local storage
    if(size <= 0){
        return NULL;
    }
    else{
        //Requested size is greater than zero
        size_t aSize = align(size);
        block my_block;
        block last_block = NULL;
        if(!global_head){
            //No list structure initialised in this thread, initialise list 
            my_block = extend_heap(last_block, aSize);

            if(!my_block){
                return NULL; //extending heap failed. so, return null;
            }
            else{
                //block extended, initialise list with the block 
                global_head = my_block;
                //return address of memory after block_meta_data
                return (my_block + 1);
            }
        }
        else{
            //List structure exists, find best fit or extend heap
            last_block = global_head; //initialise last_block as head and we iterate it to the end
            
            my_block = find_best_fit_block(&last_block, aSize);

            if(my_block != NULL){
                //we can either fill the best_block or split it.
                //splitting is efficient for memory but splitting also causes fragmentation. So, blocks need to be merged with free
                if(my_block->size >= aSize + BLOCK_SIZE){
                    split_block(my_block, aSize);
                }

                my_block->free = 0;

                return (my_block + 1);
            } 
            else{
                //could not find existing block
                //extend heap and create new block
                my_block = extend_heap(last_block, aSize);

                if(!my_block){
                    return NULL; //extending heap failed. so, return null;
                }
                else{
                    return (my_block + 1);
                }
            }
        }
    }
}

void *get_block_from_pointer(void *ptr){
    return (block)ptr - 1;
}

void merge_block(block merge_block){
    while(merge_block->next != NULL){
        if(merge_block->next->free){
            merge_block->size += merge_block->next->size + BLOCK_SIZE;
            merge_block->next = merge_block->next->next;

            if(merge_block->next != NULL){
                merge_block->next->prev = merge_block;
            }
        }
        else{
            break;
        }
    }

}

void free(void *ptr){

    if(!ptr){
        return;
    }
    block block = get_block_from_pointer(ptr);
    if(block){
        block->free = 1;
        int block_prev_merge = 0;
        if(block->prev != NULL){
            if(block->prev->free){
                merge_block(block->prev);
                block_prev_merge = 1;
            }
        }

        if(block_prev_merge == 0){
            merge_block(block);
        }
    }

}

void *realloc(void *ptr, size_t size) {
    /*
    * Insert realloc implementation here
    */
    
    if(!ptr){
        //if NUll_PTR is given then, just return a pointer with the size
        return malloc(size);
    }

    block block = get_block_from_pointer(ptr);

    if(block->size >= size){
        //wont shrink size
        return ptr;
    }

    //find new memory
    void *new_ptr = malloc(size);

    if(!new_ptr){
        return NULL;
    }

    //copy data to new memory
    memcpy(new_ptr, ptr, block->size);

    //free old memory address
    free(ptr);

    return new_ptr;
}