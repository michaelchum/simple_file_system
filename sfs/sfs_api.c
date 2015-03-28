#include <stdio.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block super_block_cache;
inode inode_table[NUM_INODES];
unsigned char free_bitmap[BLOCK_SIZE];
file_descriptor fd_table[MAXFILES];

int cur_dir_address;

int mksfs(int fresh) {

    int super_block_buffer[BLOCK_SIZE];
    memset(super_block_buffer, 0, BLOCK_SIZE);

    int inode_table_buffer[10 * BLOCK_SIZE];
    memset(inode_table_buffer, 0, sizeof(inode_table_buffer)); 

    char dir_buffer[5 * BLOCK_SIZE];
    memset(dir_buffer, 0, sizeof(dir_buffer));

    if (fresh) {
        init_fresh_disk("FILESYSTEM", BLOCK_SIZE, NUM_BLOCKS);
        init_disk("FILESYSTEM", BLOCK_SIZE, NUM_BLOCKS);

        // Initialize super block
        super_block super_block_cache = {
            .magic_number = 0xAABB0005,
            .block_size = BLOCK_SIZE,
            .file_system_size = NUM_BLOCKS,
            .inode_table_length = NUM_BLOCKS,
            .root_dir = 0 
        };
        // Copy super block to buffer
        memcpy((void *)super_block_buffer, (const void *) &super_block_cache, sizeof(super_block));
        // Write super block as first block on disk
        write_blocks(0, 1, super_block_buffer);

        // Initialize i-nodes
        inode root_inode = {
            .mode=0766, 
            .link_count=1,
            .uid=0,
            .gid=0,
            .size=0,
            .pointers={17,18,19,20,21,0,0,0,0,0,0,0,0}
        };

        // Set inode table to Os
        memset(inode_table, 0, sizeof(inode_table));
        // Set root i-node into table
        inode_table[super_block_cache.root_dir] = root_inode;
        // Copy inode table to buffer
        memcpy((void *)inode_table_buffer, &inode_table, sizeof(inode_table)); 
        // Write inode table to disk
        write_blocks(1, 10, inode_table_buffer);

        // Initialize root dir
        memset(root_dir, 0, sizeof(root_dir));
        // Copy root dir to buffer
        memcpy((void *)dir_buffer, (const void *)&root_dir, sizeof(root_dir));
        // Write root dir to disk
        write_blocks(11, 5, dir_buffer);

        // Initialize free space bitmap
        memset(free_bitmap, 0, sizeof(free_bitmap));
        free_bitmap[0] = 0b11111111; // block 0 = super block, block 1-7 = inode_table
        free_bitmap[1] = 0b11111111; // block 8-10 = inode_table, block 11-15 = root dir
        free_bitmap[BLOCK_SIZE-1] = 0b00000001; // last block = free_bitmap
        // Write free space bitmap to disk
        write_blocks(NUM_BLOCKS-1, 1, free_bitmap);

        // Initialize file descriptor table on memory not disk
        memset(fb_table, 0, sizeof(fd_table));

        // Set dir pointer to root dir
        cur_dir_address = 0;

    } else {
        init_disk("FILESYSTEM", BLOCK_SIZE, NUM_BLOCKS);
    }
    // void *ptr;
    // char input = 'c';
    // char *output;
    // write_blocks(0, 1, &input);
    // output = malloc(1024);
    // read_blocks(0, 1, output);
    // printf("%c\n",output[0]);
    return 0;
}

int sfs_fopen(char *name) {
    return 0;
}

int sfs_fclose(int fileID) {
    return 0;
}

int sfs_fwrite(int fileID, const char *buf, int length) {
    return 0;
}

int sfs_fread(int fileID, char *buf, int length) {
    return 0;
}

int sfs_fseek(int fileID, int offset) {
    return 0;
}

int sfs_remove(char *file) {
    return 0;
}

int sfs_get_next_filename(char* filename) {
    return 0;
}

int sfs_GetFileSize(const char* path) {
    return 0;
}

// int main(int argc, char const *argv[]) {
//     /* code */
//     mksfs(1);
//     return 0;
// }
