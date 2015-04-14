#include <stdio.h>      
#include <stdlib.h>

#include "my_malloc.h"

int main(int argc, char *argv[]) {
    // block *head = malloc(sizeof(block));
    // printf("*head size %lu\n", sizeof(head));
    // printf("int size %lu\n", sizeof(int));

    void* new_data_ptr = my_malloc(100);

    void* new_data_ptr2 = my_malloc(1000);

    void* new_data_ptr3 = my_malloc(1000);

    // my_free(new_data_ptr);
    // my_free(new_data_ptr3);
    my_free(new_data_ptr2);

    void* new_data_ptr4 = my_malloc(200);

	my_mallinfo();

    return 0;
}