#ifndef BWFS_IO_H
#define BWFS_IO_H

#include "bwfs.h"

int bwfs_save_image(const char *path, bwfs_superblock *sb);
int bwfs_load_image(const char *path, bwfs_superblock *sb);

#endif
