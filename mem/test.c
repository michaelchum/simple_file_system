#include <stdio.h>      
#include <stdlib.h>

#include "my_malloc.h"

int main(int argc, char *argv[]) {
    // block *head = malloc(sizeof(block));
    // printf("*head size %lu\n", sizeof(head));
    // printf("int size %lu\n", sizeof(int));

    void* new_data_ptr = my_malloc(100);
    printf("Free data 1 pointed at %td\n", new_data_ptr);

    void* new_data_ptr2 = my_malloc(1000);
    
    my_free(new_data_ptr2);
	
	// void* new_data_ptr2 = my_malloc(1000);
 //    printf("Free data 2 pointed at %td\n", new_data_ptr2);

    // block block1 = { 0, NULL, NULL, NULL };
    // block block2 = { 0, NULL, NULL, NULL };
    // block block3 = { 0, NULL, NULL, NULL };

    // block *block1ptr = &block1;
    // block *block2ptr = &block2;
    // block *block3ptr = &block3;

    // block1ptr->next_block = block2ptr;
    // block2ptr->next_block = block3ptr;

    // printf("block1ptr %p\n", block1ptr);
    // printf("block2ptr %p\n", block2ptr);
    // printf("block3ptr %p\n", block3ptr);

    // block *test = block1ptr;
    // printf("test %p\n", test);
    // test = test->next_block;
    // printf("test %p\n", test);
    // printf("block1ptr %p\n", block1ptr);

    return 0;
}