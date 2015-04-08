#include <stdio.h>      
#include <stdlib.h>  
#include <unistd.h>
#include "mymalloc.h"

int current_policy = FIRST_FIT; // default policy FIRST_FIT
int bytes_allocated = 0;
const int buffer_size = 128*1024;
block *head = (const char *)&free_list;

// Find a free block according to the currently selected policy
void *find_free_block(int size) {
    // Pointer to point to the head of the free_list
    block *ptr = head;
    // FIRST FIT Iterate through free_list, return the first fitting block
    if (current_policy == FIRST_FIT) {
        while (ptr != NULL) {
            if (ptr->length >= size) {
                return head;
            } else {
                ptr = ptr->next_block;
            }
        }
        return NULL;
    // BEST FIT Iterate thought free_list, return the smallest fitting block
    } else {
        int free_block_found = 0;
        block *smallest_block = head;
        while (ptr->next_block != NULL) {
            if (ptr->length >= size) {
                if (ptr->length < smallest_block->length) {
                    smallest_block = ptr;
                }
                free_block_found = 1;
            }
        }
        if (free_block_found) {
            return smallest_block;
        } else {
            return NULL;
        }
    }
}

void my_mallopt(int policy) {
    if (policy != FIRST_FIT || policy != BEST_FIT) {
        printf("Error - my_mallopt - Wrong policy input\n");
        return;
    }
    current_policy = policy;
}

void *my_malloc(int size) {

}

void my_free(void *ptr) {

}

void my_mallopt(int policy) {

}

void my_mallinfo() {
    
}



