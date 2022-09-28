# Server Client Programs using Shared Memory Buffers

Compile using make:
Run server: ./server <hashtable_size>
Run client: ./client

# Server

Server initialises a Hashtable and attaches a shared memory buffer. Then it waits for incoming requests by iterating over the requests structure stored in the shared memory buffer. 

# Client

Client also attaches a shared memory buffer. It can send requests by writing into the request structs in the shared memory buffer. And receives response as they are processed by the Server. 

Available commands: INSERT, READ and DELETE
after entering command and value can be entered. The value is then stored in the hashtable by the Server. 

# Locks
we use locks to ensure concurrency while editing the hashtable and the request struct. 

