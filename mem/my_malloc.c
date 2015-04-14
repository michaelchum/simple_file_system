#include <stdio.h>      
#include <stdlib.h>  
#include <unistd.h>
#include "my_malloc.h"

int current_policy = FIRST_FIT; // default policy FIRST_FIT
int bytes_allocated = 0;
const int buffer_size = 10240000; // 128 KBytes = 1,024,000 bits
const int block_size = 8*sizeof(block); // size of the header block struct in bit
void *start;
block *free_list_head;

// Find a free block according to the currently selected policy
void *find_free_block(int size) {
    // Check if free_list_head has been initialized
    if (free_list_head == NULL) {
        return NULL;
    }
    // Pointer to point to the free_list_head of the free_list
    block *ptr = free_list_head;
    // FIRST FIT Iterate through free_list, return the first fitting block
    if (current_policy == FIRST_FIT) {
        while (ptr != NULL) {
            if (ptr->length >= size) {
                return ptr;
            } else {
                ptr = ptr->next_free;
            }
        }
        return NULL;
    // BEST FIT Iterate thought free_list, return the smallest fitting block
    } else {
        int free_block_found = 0;
        block *smallest_block = NULL;
        while (ptr != NULL) {
            if (ptr->length >= size) {
                if (smallest_block == NULL) {
                    smallest_block = ptr;
                } else if (ptr->length < smallest_block->length) {
                    smallest_block = ptr;
                }
                free_block_found = 1;
            }
            if (ptr->next_free != NULL) {
                ptr = ptr->next_free;
            } else if (free_block_found) {
                return smallest_block;
            }
        }
        return NULL;
    }
}

void my_mallopt(int policy) {
    if (policy != FIRST_FIT || policy != BEST_FIT) {
        printf("Error - my_mallopt - Wrong policy input\n");
        return;
    }
    current_policy = policy;
}

void *my_malloc(int size_byte) {
    // Convert size into bits
    int size_bit = 8*size_byte;
    printf("my_malloc() is trying to allocate %i bit...\n", size_bit);
    // Check if free_list_head has not been initialized
    if (free_list_head == NULL) {
        printf("No free_list_head found, initializing free_list...\n");
        // Set the initial program break
        free_list_head = sbrk(0); // IMPORTANT sbrk() on Ubuntu doesn't behave the same as sbrk() on OS X
        // Store beginning of memory
        start = sbrk(0);
        brk((void *)free_list_head + block_size + buffer_size);
        // Create first block
        *free_list_head = (block){ 0, NULL, NULL, NULL, 1, NULL };
        free_list_head->length = buffer_size;
        free_list_head->free_data = (void *)free_list_head + block_size;

        printf("Size of a block free_list_header is %lu bit\n", block_size);
        printf("free_list_head starts at %td\n", free_list_head);
        printf("free_list_head's free data starts at %td\n", free_list_head->free_data);
        printf("free_list_head's length is %i bit\n", free_list_head->length);
    }

    // Find free block
    block *initial_free_block = find_free_block(size_bit+block_size);
    // Could not find free block, expand program break
    if (initial_free_block == NULL) {
        block *ptr = free_list_head;
        // Go to the last block of the free_list
        while (ptr->next_free != NULL) {
            ptr = ptr->next_free;
        }
        // Expand last free block until size is reached
        while (find_free_block(size_bit+block_size) == NULL) {
            ptr->length = ptr->length + buffer_size;
            sbrk(buffer_size);
        }
        // Enough space now
        initial_free_block = ptr;
    }

    printf("Initial free block address: %td\n", initial_free_block);
    printf("Initial free block length: %i bit\n", initial_free_block->length);

    // Split the initial free block into 
    // 1. New data block of length size
    // 2. New free free block of length (initial_free_block length) - size
    block *new_data_block;
    block *new_free_block;

    // Locate new_free_block
    if (initial_free_block == free_list_head) {
        // Move the head
        free_list_head = (void *)(free_list_head->free_data + size_bit);
        printf("New free_list_head points at %td\n", free_list_head);
        new_free_block = free_list_head;
    } else {
        new_free_block = (void *)(initial_free_block->free_data + size_bit);
    }
    // Create a new free block
    *new_free_block = (block){ 0, NULL, NULL, NULL, 1, NULL };

    new_free_block->is_free = 1;
    new_free_block->prev_block = initial_free_block;
    new_free_block->next_block = initial_free_block->next_block;
    new_free_block->free_data = (void *)new_free_block + block_size;
    new_free_block->length = initial_free_block->length - size_bit - block_size;

    // NEXT_BLOCK IS NOT FREE
    if (new_free_block->next_block != NULL && ((new_free_block->next_block)->is_free == 0)) {
        new_free_block->next_free = initial_free_block->next_free;
    // NEXT_BLOCK IS FREE, MERGE
    } else if (new_free_block->next_block != NULL) {
        new_free_block->next_free = (new_free_block->next_block)->next_free;
        new_free_block->length = initial_free_block->length - size_bit + (new_free_block->next_block)->length;
        new_free_block->next_block = (new_free_block->next_block)->next_block;
    }

    // Locate new_data_block
    new_data_block = initial_free_block;
    new_data_block->is_free = 0;
    new_data_block->length = size_bit;
    new_data_block->next_block = new_free_block;
    new_data_block->next_free = new_free_block;

    // REDIRECT all prev blocks' next free block to this new free block
    block *prev = (block *)start;
    while (prev < new_free_block) {
        if (prev->next_free == initial_free_block) {
            prev->next_free = new_free_block;
        }
        prev = prev->next_block;
    }

    bytes_allocated += size_byte;

    printf("New fragmented data block address: %td\n", new_data_block);
    printf("New fragmented data block length: %i bit\n", new_data_block->length);
    printf("New fragmented free block address: %td\n", new_free_block);
    printf("New fragmented free block length: %i bit\n", new_free_block->length);

    return new_data_block->free_data;
}

// Takes a pointer to a block previously allocated by my_malloc() and deallocate it.

void my_free(void *input_ptr) {

    // If input is NULL, should not free anything
    if (input_ptr == NULL) {
        return;
    }
    // Note that ptr currently points to the data block
    // Point it to the header block
    input_ptr = (void *)(input_ptr - block_size);
    block *ptr = (block *)input_ptr;

    int prev_free = 0;
    int next_free = 0;
    block *prev_block = ptr->prev_block;
    block *next_block = ptr->next_block;

    // Check if previous block is free
    if (prev_block != NULL && prev_block->is_free) {
        prev_free = 1;
    }

    // Check if next block is free
    if (next_block != NULL && next_block->is_free) {
        next_free = 1;
    }

    int bytes_deallocated = 0;
    block *new_free_block;

    // Both sides free, merge
    if (prev_free && next_free) {
        printf("Both prev and next block are free, merge!\n");
        bytes_deallocated = 2*block_size + ptr->length;
        // New free block
        new_free_block = prev_block;
        // Calculate the new length
        int new_length = prev_block->length + 2*block_size + ptr->length + next_block->length;
        
        new_free_block->is_free = 1;
        new_free_block->length = new_length;
        new_free_block->next_block = next_block->next_block;
        new_free_block->next_free = next_block->next_free;

        // REDIRECTION All block previously pointing at next_free now points at new_free_block (also prev_block)
        block *prev = (block *)start;
        while (prev < new_free_block) {
            if (prev->next_free == next_block) {
                prev->next_free = new_free_block;
            }
            prev = prev->next_block;
        }

    // Previous block free, merge
    } else if (prev_free) {
        printf("Only prev block is free, merge!\n");
        bytes_deallocated = block_size + ptr->length;
        // New free block
        new_free_block = prev_block;       
        // Calculate the new length
        int new_length = prev_block->length + block_size + ptr->length;

        new_free_block->is_free = 1;
        new_free_block->length = new_length;
        new_free_block->next_block = next_block;
        new_free_block->next_free = next_block->next_free;

    // Next block free, merge
    } else if (next_free) {
        printf("Only next block is free, merge!\n");
        bytes_deallocated = block_size + ptr->length;
        // New free block
        new_free_block = ptr;
        // Calculate the new length
        int new_length = ptr->length + block_size + next_block->length;

        new_free_block->is_free = 1;
        new_free_block->length = new_length;
        new_free_block->next_block = next_block->next_block;
        new_free_block->next_free = next_block->next_free;

        // REDIRECTION All block previously pointing at next_free now points at new_free_block
        block *prev = (block *)start;
        while (prev < new_free_block) {
            if (prev->next_free == next_block) {
                prev->next_free = new_free_block;
            }
            prev = prev->next_block;
        }

        // Check if we need to move free_list_head
        if (free_list_head > ptr) {
            free_list_head = ptr;
        }
    // No neighbors free, need to connect free_list on both sides
    } else {
        printf("No neighbors free\n");
        bytes_deallocated = ptr->length;
        // New free block
        new_free_block = ptr;

        new_free_block->is_free = 1;
        new_free_block->next_free = next_block->next_free;

        // REDIRECTION All block previously pointing at next_block->next_free now points at new_free_block
        block *prev = (block *)start;
        while (prev < new_free_block) {
            if (prev->next_free == next_block->next_free) {
                prev->next_free = new_free_block;
            }
            prev = prev->next_block;
        }

        // Check if we need to move free_list_head
        if (free_list_head > ptr) {
            free_list_head = ptr;
        }
    }

    block *last_block = free_list_head;
    // Go to the last block of the free_list
    while (last_block->next_block != NULL) {
        last_block = last_block->next_free;
    }
    // CHECK whether the last block is larger than 128KBytes and shrink
    if (last_block->length > buffer_size) {
        last_block->length = buffer_size;
        brk((void*)last_block->free_data + buffer_size);
    }

    bytes_allocated -= bytes_deallocated/8;
}

// Print every block of the heap
void print_heap() {
    block *itr_block = (block *)start;
    void *end = sbrk(0);
    printf("\n===============HEAP MAP===============\n");
    while (itr_block < end) {
        if (itr_block->is_free) {
            printf("*** Free block ***");
            if (itr_block == free_list_head) {
                printf(" <<< FREE LIST HEAD");
            }
            printf("\n");
        } else {
            printf("*** Allocated block ***\n");  
        }
        printf("Address: %td\n", itr_block);
        printf("Length: %i bit\n", itr_block->length);
        printf("Prev block: %td\n", itr_block->prev_block);
        printf("Next block: %td\n", itr_block->next_block);
        printf("Next free block: %td\n", itr_block->next_free);
        itr_block = (block *)((void *)itr_block->free_data + itr_block->length);
    }
    printf("===============END MAP===============\n\n");
}

void my_mallinfo() {

    printf("\n\n===========MEMORY ALLOCATION INFO===========\n");

    // Largest contiguous free block
    int largest_free_block = free_list_head->length;

    // Total free space
    int total_free_space = 0;

    block *itr_block = free_list_head;
    while (itr_block != NULL) {
        total_free_space += itr_block->length;
        if (itr_block->length > largest_free_block) {
            largest_free_block = itr_block->length;
        }
        itr_block = itr_block->next_free;
    }

    printf("Allocated                     \t%i Bytes\n", bytes_allocated);
    printf("Total free space              \t%i Bytes\n", total_free_space/4);
    printf("Largest contiguous free block \t%i Bytes\n", largest_free_block/4);

    // Print a map of the allocated heap
    print_heap();
}



