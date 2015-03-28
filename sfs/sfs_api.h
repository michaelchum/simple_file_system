#define MAXFILENAME 20
#define MAXFILEEXTENSION 3
#define BLOCK_SIZE 512 // bytes
#define NUM_BLOCKS 4096 
#define NUM_INODES 100
#define NUM_INODE_POINTERS 13 // 12 data blocks pointers, 1 single indirect pointer
#define MAX_NUM_FILES 99
#define MAX_FILESIZE 4096 // bytes

typedef struct {
    int magic_number;
    int block_size; // # of bytes 
    int file_system_size; // # of blocks
    int inode_table_length; // # of blocks
    int root_dir; // ID of the inode pointing to root directory
} super_block;

typedef struct {
    int mode; 
    int link_count;
    int uid;
    int gid;
    int size;
    int pointers[NUM_INODE_POINTERS];
} inode;

typedef struct{
    int inode_id;
    char file_name[MAXFILENAME+MAXFILEEXTENSION];
    char name[MAXFILENAME];
    char extension[MAXFILEEXTENSION];
} dir;

typedef struct{
    int opened;
    int rw_ptr;
} file_descriptor;

// API Declaration

int mksfs(int fresh);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fseek(int fileID, int offset);
int sfs_remove(char *file);
int sfs_get_next_filename(char* filename);
int sfs_GetFileSize(const char* path);