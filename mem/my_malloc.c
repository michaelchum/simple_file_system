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
                ptr = ptr->next_block;
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
            if (ptr->next_block != NULL) {
                ptr = ptr->next_block;
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
        start = free_list_head;
        brk((void *)free_list_head + block_size + buffer_size);
        // Create first block
        *free_list_head = (block){ 0, NULL, NULL, NULL };
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
        while (ptr->next_block != NULL) {
            ptr = ptr->next_block;
        }
        // Expand last free block until size is reached
        while (find_free_block(size_bit+block_size) == NULL) {
            ptr->length = ptr->length + buffer_size;
            sbrk(buffer_size);
        }
        // Enough space now
        initial_free_block = ptr;
    }
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
    // Create a new block to be our new fragmented free_list
    *new_free_block = (block){0, NULL, NULL, NULL};

    printf("Initial free block address: %td\n", initial_free_block);
    printf("Initial free block length: %i bit\n", initial_free_block->length);

    new_free_block->prev_block = initial_free_block;
    new_free_block->next_block = initial_free_block->next_block;
    new_free_block->length = initial_free_block->length - size_bit - block_size;
    new_free_block->free_data = (void *)new_free_block + block_size;

    // Locate new_data_block
    new_data_block = initial_free_block;
    new_data_block->length = size_bit;
    new_data_block->next_block = new_free_block;

    if (new_data_block->prev_block != NULL) {
        (new_data_block->prev_block)->next_block = new_free_block;
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
    // Note that ptr currently points to the data_block
    // Point it to the header_block
    input_ptr = (void *)(input_ptr - block_size);

    block *ptr = (block *)input_ptr;
    int bytes_deallocated = ptr->length/8;

    printf("Deallocating %i bytes...\n", bytes_deallocated);

    int prev_free = 0;
    int next_free = 0;

    // Check if previous block is free
    // Iterate through free_list and check if the physical prev block is actually a free block
    block *physical_prev_block = free_list_head;
    void *next_to_free = NULL;
    while (physical_prev_block != NULL) {
        next_to_free = (void *)(physical_prev_block->free_data + physical_prev_block->length);
        if (next_to_free == ptr) {
            prev_free = 1;
            break;
        }
        physical_prev_block = physical_prev_block->next_block;
    }

    // Check if next block is free
    // A ptr->next_block ALWAYS point to the next free block according to our my_malloc() implementation
    // So if the physical next block is the same as ptr->next_block, the next block is indeed free
    block *physical_next_block = (void *)(ptr->free_data + ptr->length);
    if (ptr->next_block == physical_next_block) {
        next_free = 1;
    }

    // Both sides free, merge
    if (prev_free && next_free) {
        printf("Both prev and next block are free, merge!\n");
        // Calculate the new length
        int new_length = physical_prev_block->length + 2*block_size + ptr->length + physical_next_block->length;
        physical_prev_block->length = new_length;
        // Point the prev block's next pointer to the next block's next pointer which is always a free block
        physical_prev_block->next_block = physical_next_block->next_block;
    // Previous block free, merge
    } else if (prev_free) {
        printf("Only prev block is free, merge!\n");
        // Calculate the new length
        int new_length = physical_prev_block->length + block_size + ptr->length;
        physical_prev_block->length = new_length;
        // Point the prev block's next pointer to the current block's next block which is always a free block
        physical_prev_block->next_block = ptr->next_block;
    // Next block free, merge
    } else if (next_free) {
        printf("Only next block is free, merge!\n");

        // Calculate the new length
        int new_length = ptr->length + block_size + physical_next_block->length;
        ptr->length = new_length;
        // Point ptr->next to next block's next free block
        ptr->next_block = physical_next_block->next_block;

        // Find the the prev free_block pointing at ptr->next_block which is always a free_block
        block *itr = free_list_head;
        while (itr != NULL) {
            if (itr->next_block == ptr->next_block) {
                physical_prev_block = itr;
                break;
            }
            itr = itr->next_block;
        }

        // Point the prev free block's next to this block
        physical_prev_block->next_block = ptr;

        // REDIRECTION Make sure all blocks between physical_prev_block to ptr, point to ptr
        block *redir;
        // If all prev blocks are data blocks
        if (free_list_head > ptr) {
            redir = (block *)start;
        // If prev free block is existant
        } else {
            redir = physical_prev_block;
        }
        while (redir < ptr) {
            redir->next_block = ptr;
            redir = (void *)redir->free_data + redir->length;
        }

        // Check if we need to move free_list_head
        if (free_list_head > ptr) {
            free_list_head = ptr;
        }
    // No neighbors free, need to connect free_list on both sides
    } else {
        printf("No neighbors free\n");

        // Find the the prev free_block pointing at ptr->next_block which is always a free_block
        block *itr = free_list_head;
        while (itr != NULL) {
            if (itr->next_block == ptr->next_block) {
                physical_prev_block = itr;
                break;
            }
            itr = itr->next_block;
        }

        // REDIRECTION Make sure all blocks between physical_prev_block to ptr, point to ptr
        block *redir;
        // If all prev blocks are data blocks
        if (free_list_head > ptr) {
            redir = (block *)start;
        // If prev free block is existant
        } else {
            redir = physical_prev_block;
        }
        while (redir < ptr) {
            redir->next_block = ptr;
            redir = (void *)redir->free_data + redir->length;
        }

        // Check if we need to move free_list_head
        if (free_list_head > ptr) {
            ptr->next_block = free_list_head;
            free_list_head = ptr;
        }
    }

    // CHECK whether the last block is larger than 128KBytes and shrink
    block *last_block = free_list_head;
    // Go to the last block of the free_list
    while (last_block->next_block != NULL) {
        last_block = last_block->next_block;
    }
    if (last_block->length > buffer_size) {
        last_block->length = buffer_size;
        brk((void*)last_block->free_data + buffer_size);
    }
}

void my_mallinfo() {

    // Total bytes allocated

    // Largest contiguous free block

    // Total free space

    printf("\n*** Memory Allocation Info ***\n");
    printf("Allocated                \t%i Bytes\n", bytes_allocated);
    printf("Free space               \t%i Bytes\n", bytes_allocated);
    printf("Largest Contiguous block \t%i Bytes\n\n", bytes_allocated);
}



