#define FIRST_FIT 1
#define BEST_FIT 2

extern char free_list;
extern char *my_malloc_error;

extern void *my_malloc(int size);
extern void my_free(void *ptr);
extern void my_mallopt(int policy);
extern void my_mallinfo();

typedef struct block {
    int length;
    struct block *previous_block;
    struct block *next_block;
    unsigned char *buffer;
} block;