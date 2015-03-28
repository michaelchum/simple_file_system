#include <stdio.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block super_block_cache;
inode inode_table[NUM_INODES];
unsigned char free_bitmap[BLOCK_SIZE];
file_descriptor fd_table[MAX_NUM_FILES];
directory root_dir[MAX_NUM_FILES]; // 2560 Bytes

int cur_dir_address;

int mksfs(int fresh) {

    unsigned char super_block_buffer[BLOCK_SIZE];
    memset(super_block_buffer, 0, BLOCK_SIZE);

    unsigned char inode_table_buffer[16 * BLOCK_SIZE];
    memset(inode_table_buffer, 0, sizeof(inode_table_buffer)); 

    unsigned char dir_buffer[14 * BLOCK_SIZE];
    memset(dir_buffer, 0, sizeof(dir_buffer));

    printf("int size %lu\n", sizeof(int));
    printf("unsigned char size %lu\n", sizeof(unsigned char));
    printf("char size %lu\n\n", sizeof(char));

    printf("Super block size %lu\n", sizeof(super_block));
    printf("I-node size %lu\n", sizeof(inode));
    printf("I-node table size %lu\n", sizeof(inode_table));
    printf("Free bitmap size %lu\n", sizeof(free_bitmap));
    printf("FD table size %lu\n", sizeof(fd_table));
    printf("Root dir size %lu\n\n", sizeof(root_dir));

    printf("Super block buffer size %lu\n", sizeof(super_block_buffer));
    printf("I-node table buffer size %lu\n", sizeof(inode_table_buffer));
    printf("Dir buffer size %lu\n", sizeof(dir_buffer));

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

        // Initialize root i-node
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
        write_blocks(1, 16, inode_table_buffer);

        // Initialize root dir
        memset(root_dir, 0, sizeof(root_dir));
        // Copy root dir to buffer
        memcpy((void *)dir_buffer, (const void *)&root_dir, sizeof(root_dir));
        // Write root dir to disk
        write_blocks(17, 14, dir_buffer);

        // Initialize free space bitmap
        memset(free_bitmap, 0, sizeof(free_bitmap));
        free_bitmap[0] = 0b11111111; // block 0 = super_block, block 1-7 = inode_table
        free_bitmap[1] = 0b11111111; // block 8-15 = inode_table
        free_bitmap[2] = 0b11111111; // block 16 = inode_table, block 17-23 = root_dir
        free_bitmap[3] = 0b11111111; // block 24-31 = root_dir
        free_bitmap[BLOCK_SIZE-1] = 0b00000001; // last block = free_bitmap
        // Write free space bitmap to disk
        write_blocks(NUM_BLOCKS-1, 1, free_bitmap);

        // Initialize file descriptor table on memory not disk
        memset(fd_table, 0, sizeof(fd_table));

        // Set dir pointer to root dir
        cur_dir_address = 0;

    } else {
        
        init_disk("FILESYSTEM", BLOCK_SIZE, NUM_BLOCKS);

        // Load super block to memory
        read_blocks(0, 1, (void *)&super_block_cache);

        // Load inode table to memory
        read_blocks(1,10, inode_table_buffer);
        int i;
        for (i = 0; i < NUM_INODES; i++) {
            memcpy((void *)&(inode_table[i]), (const void *)(inode_table_buffer + i*(sizeof(inode)/4)), sizeof(inode));
        }

        // Load free bitmap to memory
        memset(free_bitmap, 0, sizeof(free_bitmap));
        read_blocks(NUM_BLOCKS-1, 1, free_bitmap);

        // Load file descriptor table to memory
        memset(fd_table, 0, sizeof(fd_table));

    }
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
