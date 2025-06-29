#ifndef BWFS_IO_H
#define BWFS_IO_H

#include "bwfs.h"

// Guarda el superbloque binario (FS_0.png)
int bwfs_save_image(const char *path, const bwfs_superblock *sb);

// Carga el superbloque binario (FS_0.png)
int bwfs_load_image(const char *path, bwfs_superblock **sb_ptr);

#endif // BWFS_IO_H
