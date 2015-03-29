#include <stdio.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block super_block_cache;
inode inode_table[NUM_INODES];
unsigned char free_bitmap[BLOCK_SIZE];
file_descriptor fd_table[MAX_NUM_FILES];
file_meta root_dir_table[MAX_NUM_FILES]; // 2560 Bytes

int cur_dir_address;

int mksfs(int fresh) {

    unsigned char super_block_buffer[BLOCK_SIZE];
    memset(super_block_buffer, 0, BLOCK_SIZE);

    unsigned char inode_table_buffer[16 * BLOCK_SIZE];
    memset(inode_table_buffer, 0, sizeof(inode_table_buffer)); 

    unsigned char dir_buffer[12 * BLOCK_SIZE];
    memset(dir_buffer, 0, sizeof(dir_buffer));

    printf("int size %lu\n", sizeof(int));
    printf("unsigned char size %lu\n", sizeof(unsigned char));
    printf("char size %lu\n\n", sizeof(char));

    printf("Super block size %lu\n", sizeof(super_block));
    printf("I-node size %lu\n", sizeof(inode));
    printf("I-node table size %lu\n", sizeof(inode_table));
    printf("Free bitmap size %lu\n", sizeof(free_bitmap));
    printf("File meta size %lu\n\n", sizeof(file_meta));
    printf("FD table size %lu\n", sizeof(fd_table));
    printf("Root dir size %lu\n\n", sizeof(root_dir_table));

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
            .root_dir_inode_index = 0 
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
            .pointers={17,18,19,20,21,22,23,24,25,26,27,28,0}
        };
        // Set inode table to Os
        memset(inode_table, 0, sizeof(inode_table));
        // Set root i-node into table
        inode_table[super_block_cache.root_dir_inode_index] = root_inode;
        // Copy inode table to buffer
        memcpy((void *)inode_table_buffer, &inode_table, sizeof(inode_table)); 
        // Write inode table to disk
        write_blocks(1, 16, inode_table_buffer);

        // Initialize root dir
        memset(root_dir_table, 0, sizeof(root_dir_table));
        // Copy root dir to buffer
        memcpy((void *)dir_buffer, (const void *)&root_dir_table, sizeof(root_dir_table));
        // Write root dir to disk
        write_blocks(17, 12, dir_buffer);

        // Initialize free space bitmap
        memset(free_bitmap, 0, sizeof(free_bitmap));
        free_bitmap[0] = 0b11111111; // block 0 = super_block, block 1-7 = inode_table
        free_bitmap[1] = 0b11111111; // block 8-15 = inode_table
        free_bitmap[2] = 0b11111111; // block 16 = inode_table, block 17-23 = root_dir_table
        free_bitmap[3] = 0b11111000; // block 24-28 = root_dir_table
        free_bitmap[BLOCK_SIZE-1] = 0b00000001; // last block = free_bitmap
        // Write free space bitmap to disk
        write_blocks(NUM_BLOCKS-1, 1, free_bitmap);

    } else {

        init_disk("FILESYSTEM", BLOCK_SIZE, NUM_BLOCKS);

        // Load super block to memory
        read_blocks(0, 1, (void *)&super_block_cache);

        // Load inode table to memory
        read_blocks(1,16, inode_table_buffer);
        int i;
        for (i = 0; i < NUM_INODES; i++) {
            memcpy((void *)&(inode_table[i]), (const void *)(inode_table_buffer+i*sizeof(inode)), sizeof(inode));
        }

        // Load free bitmap to memory
        memset(free_bitmap, 0, sizeof(free_bitmap));
        read_blocks(NUM_BLOCKS-1, 1, free_bitmap);

        // Load root dir to memory
        inode root_inode = inode_table[super_block_cache.root_dir_inode_index];
        int j, num_block;
        // Here, by design, we assume all pointers of root_inode point to subsequent blocks following eachother, this is only true for the root_inode
        // Also, there is no indirect pointer in the root_inode
        for (j = 0; j < NUM_INODE_POINTERS; j++) {
            if (root_inode.pointers[j] != 0) {
                num_block++;
            }
        }
        // Load to memory
        read_blocks(root_inode.pointers[0], num_block, dir_buffer);
        int k;
        for (k = 0; k < MAX_NUM_FILES; k++) {
            memcpy((void *)&(root_dir_table[k]), (const void *)(dir_buffer+k*sizeof(file_meta)), sizeof(file_meta));
        }

    }

    // Initialize file descriptor table on memory
    memset(fd_table, 0, sizeof(fd_table));

    // Set dir pointer to root dir
    cur_dir_address = 0;

    return 0;
}

int sfs_fopen(char *name) {

    // Check if file exists
    int i;
    for (i = 0; i < MAX_NUM_FILES; i++) {
        // File exists
        if (strncmp(root_dir_table[i].file_name, name, MAXFILENAME) == 0) {
            if (fd_table[i].opened == 1) {
                return -1; // File already opened
            }
            // Open the file
            fd_table[i].opened = 1; 
            // Load the inode corresponding to the file
            inode file_inode = inode_table[root_dir_table[i].inode_index];
            // Check the 13th indirect pointer
            if (file_inode.pointers[NUM_INODE_POINTERS-1] != 0) {
                // Load the indirect pointer block
                int indirect_pointers_buffer[BLOCK_SIZE/4];
                read_blocks(file_inode.pointers[NUM_INODE_POINTERS-1], 1, indirect_pointers_buffer);
                int k;
                for (k = 0; k < BLOCK_SIZE/4; k++) {
                    // Find last empty block
                    if (indirect_pointers_buffer[k] == 0) {
                        // Load last non-empty block
                        unsigned char block_buffer[BLOCK_SIZE];
                        read_blocks(indirect_pointers_buffer[k-1], 1, block_buffer);
                        // Find the last non-empty byte
                        int byte_pointer = BLOCK_SIZE-1;
                        while (block_buffer[byte_pointer] == 0) {
                            byte_pointer--;
                        }
                        // The byte number of the end of the file starting from the start of the disk
                        fd_table[i].rw_ptr = byte_pointer + 1 + (file_inode.pointers[NUM_INODE_POINTERS-1] - 1) * BLOCK_SIZE;
                        // Return the index of the file
                        printf("Opened on root dir index %i\n", i);
                        return i;
                    }
                }
            }
            // Search the first 12 direct pointers for the end of the file
            int j;
            for (j = 0; j < NUM_INODE_POINTERS-1; j++) {
                // Find last empty block
                if (file_inode.pointers[j] == 0) {
                    // Load last non-empty block
                    unsigned char block_buffer[BLOCK_SIZE];
                    read_blocks(file_inode.pointers[j-1], 1, block_buffer);
                    // Find the last non-empty byte
                    int byte_pointer = BLOCK_SIZE-1;
                    while (block_buffer[byte_pointer] == 0) {
                        byte_pointer--;
                    }
                    // The byte number of the end of the file starting from the start of the disk
                    fd_table[i].rw_ptr = byte_pointer + 1 + (file_inode.pointers[j-1] - 1) * BLOCK_SIZE;
                    // Return the index of the file
                    printf("Opened on root dir index %i\n", i);
                    return i;
                }
            }

        }
    }

    // File does not exist, create it

    // Find empty file_meta in root_dir_table, assume there will always be one
    int h;
    int new_file_index;
    for (h = 0; h < MAX_NUM_FILES; h++) {
        if (root_dir_table[h].inode_index == 0) {
            new_file_index = h;
            break;
        }
    }
    // Copy the file name to the new file_meta
    file_meta new_file;
    strcpy(new_file.file_name, name);

    // Find empty inode in inode_table, assume there will always be one
    int l;
    int new_inode_index;
    for (l = 1; l < NUM_INODES; l++) {
        if (inode_table[l].link_count == 0) {
            new_inode_index = 1;
            break;
        }
    }
    inode new_inode = {
        .mode = 0766,
        .link_count = 1,
        .uid = 0,
        .gid = 0,
        .size = 0,
        .pointers = {0,0,0,0,0,0,0,0,0,0,0,0,0}
    };

    // Store everything in memory
    inode_table[new_inode_index] = new_inode;
    new_file.inode_index = new_inode_index;
    root_dir_table[new_file_index] = new_file;

    // Write updated inode_table to disk
    unsigned char inode_table_buffer[16 * BLOCK_SIZE];
    memset(inode_table_buffer, 0, sizeof(inode_table_buffer));
    memcpy((void *)inode_table_buffer, (const void *)&inode_table, sizeof(inode_table));
    write_blocks(1, 16, inode_table_buffer);

    // Write updated root_dir_table to disk
    unsigned char dir_buffer[12 * BLOCK_SIZE];
    memset(dir_buffer, 0, sizeof(dir_buffer));
    memcpy((void *)dir_buffer, (const void *)&root_dir_table, sizeof(root_dir_table));
    write_blocks(17, 12, dir_buffer);

    printf("Created on root dir index %i\n", new_file_index);

    // Return the index of the file
    return new_file_index;
}

int sfs_fclose(int file_index) {

    if (fd_table[file_index].opened == 1) {
        fd_table[file_index].opened = 0;
        printf("Closed root dir index %i\n", file_index);
    }

    return 0;
}

int find_free_block() {

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
