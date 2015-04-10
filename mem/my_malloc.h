#define FIRST_FIT 1
#define BEST_FIT 2

typedef struct block {
    int length; // Length of the free segment
    struct block *prev_block; // Pointer to the prev free block
    struct block *next_block; // Pointer the next free block
    void *free_data; // Pointer to the start of the free segment
} block;

char *my_malloc_error;

void *my_malloc(int size);
void my_free(void *ptr);
void my_mallopt(int policy);
void my_mallinfo();