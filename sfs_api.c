#include <stdio.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block super_block_cache;
inode inodes[NUM_INODES];

int cur_dir_address;

int mksfs(int fresh) {

    int super_block_buffer[BLOCK_SIZE];
    memset(super_block_buffer, 0, BLOCK_SIZE);

    int inode_table_buffer[15 * BLOCK_SIZE];
    memset(inode_table_buffer, 0, sizeof(inode_table_buffer)); 

    char dir_buffer[5 * BLOCK_SIZE];
    memset(dir_buffer, 0, sizeof(dir_buffer));

    if (fresh) {
        init_fresh_disk("FILESYSTEM", BLOCK_SIZE, NUM_BLOCKS);
        init_disk("FILESYSTEM", BLOCK_SIZE, NUM_BLOCKS);

        super_block super_block_cache = {
            .magic_number = 0xAABB0005,
            .block_size = BLOCK_SIZE,
            .file_system_size = NUM_BLOCKS,
            .inode_table_length = NUM_BLOCKS,
            .root_directory = 0 
        };
        // Copy super block to buffer
        memcpy((void *)super_block_buffer, (const void *) &super_block_cache, sizeof(super_block));
        // Write super block as first block on disk
        write_blocks(0, 1, super_block_buffer);

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
