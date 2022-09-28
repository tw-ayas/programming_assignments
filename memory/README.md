# Malloc and free Implementations

memory.c includes the implemenation for system calls malloc, free and also realloc. sbrk() is used to extend the heap memory. The heap memory is managed with a doubly linked list of blocks. 

struct block_meta{
    size_t size;
    struct block_meta *next;
    struct block_meta *prev;
    int free;
};

For efficient use of memory, the blocks are also reused and they are split or merged. 

# Compilation

Compile with make (make all). compiles a library memory.so and program mallocfree.
Run with LD_PRELOAD=./memory.so mallocfree <threads> <iterations>
Both parameter are optional

# Allocation using Best fit strategy

For allocating memory, I have used the best fit strategy. For this strategy, we first iterate through the complete linked list to find the smallest block that fits our size. Then we iterate again to find the pointer of this best block and use it. If no such block can be found, we extend the heap and create a new block. 

# Thread safety using THREAD LOCAL STORAGE (TLS)

Concurrently operating on the same memory structure which multithreading is made safe with local storage. Each thread handles its own doubly linked list. 

__thread block global_head = NULL; //Each thread gets its own head

But we still apply a mutex on the sbrk() (not thread safe) operation as we are extending the same memory heap for all threads. 

I have also aligned the requested size. 

# Test with memory_tester.c => mallocfree

Creates n threads with m iterations (Default 1 thread and 100 iterations). With each iteration allocate memory, set the memory random value and then free the memory. 