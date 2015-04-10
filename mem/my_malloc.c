#include <stdio.h>      
#include <stdlib.h>  
#include <unistd.h>
#include "my_malloc.h"

int current_policy = FIRST_FIT; // default policy FIRST_FIT B145
int bytes_allocated = 0;
const int buffer_size = 128*1000; // 128KBytes
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
        block *smallest_block = free_list_head;
        while (ptr != NULL) {
            if (ptr->length >= size) {
                if (ptr->length < smallest_block->length) {
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
    if (policy != FIRST_FIT && policy != BEST_FIT) {
        printf("Error - my_mallopt - Wrong policy input\n");
        return;
    }
    current_policy = policy;
}

void *my_malloc(int size) {
    // Check if free_list_head has not been initialized
    if (free_list_head == NULL) {
        // Set the initial program break
        free_list_head = sbrk(0);
        brk((void *)free_list_head + sizeof(block) + buffer_size);
        // Create first block
        *free_list_head = (block){ 0, NULL, NULL, NULL };
        free_list_head->length = buffer_size;
        free_list_head->free_data = (void *)free_list_head + 8*sizeof(block);

        printf("free_list_head starts at %td\n", free_list_head);
        printf("free_list_head's free data starts at %td\n", free_list_head->free_data);
        printf("free_list_head's length is %i\n", free_list_head->length);
        printf("Size of a block free_list_header is %lu\n", sizeof(block));
    }
    // Find free block
    block *initial_free_block = find_free_block(size);
    // Could not find free block, expand program break
    if (initial_free_block == NULL) {
        block *ptr = free_list_head;
        // Go to the last block of the free_list
        while (ptr->next_block != NULL) {
            ptr = ptr->next_block;
        }
        // Expand last free block until size is reached
        while (find_free_block(size) == NULL) {
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
        free_list_head = (block *)free_list_head->free_data + size;
        new_free_block = free_list_head;
    } else {
        new_free_block = (block *)initial_free_block->free_data + size;
    }
    // Create a new block to be our new fragmented free_list
    *new_free_block = (block){0, NULL, NULL, NULL};

    printf("Initial free block length: %i\n", initial_free_block->length);
    printf("Initial free block address: %td\n", initial_free_block);

    new_free_block->prev_block = initial_free_block;
    new_free_block->next_block = initial_free_block->next_block;
    new_free_block->length = initial_free_block->length - size - sizeof(block);
    new_free_block->free_data = (void *)new_free_block + sizeof(block);

    // Locate new_data_block
    new_data_block = initial_free_block;
    new_data_block->length = size;
    new_data_block->next_block = new_free_block;

    if (new_data_block->prev_block != NULL) {
        (new_data_block->prev_block)->next_block = new_free_block;
    }

    bytes_allocated += size;

    printf("New fragmented data block length: %i\n", new_data_block->length);
    printf("New fragmented free block length: %i\n", new_free_block->length);

    return new_data_block->free_data;
}

void my_free(void *ptr) {

}

void my_mallinfo() {
    // Total bytes allocated

    // Largest contiguous free block

    // Total free space
}



