#ifndef BWFS_H
#define BWFS_H

#include <time.h>
#include <sys/types.h>

#define BWFS_MAGIC 0xBEEFBEEF
#define BWFS_BLOCK_WIDTH 1000
#define BWFS_BLOCK_HEIGHT 1000
#define BWFS_BLOCK_SIZE 4096
#define BWFS_MAX_BLOCKS 1024
#define BWFS_MAX_FILES 128
#define BWFS_FILENAME_MAX 255

typedef struct {
    int used;
    char filename[BWFS_FILENAME_MAX];
    int size;
    int blocks[10];
    mode_t mode;
    time_t atime, mtime, ctime;
} bwfs_inode;

typedef struct {
    int magic;
    int num_inodes;
    int num_blocks;
    unsigned char block_bitmap[BWFS_MAX_BLOCKS];
    bwfs_inode inodes[BWFS_MAX_FILES];
} bwfs_superblock;

#endif
