#ifndef BWFS_IO_H
#define BWFS_IO_H

#include "bwfs.h"

// Guarda el superbloque del FS en un archivo binario
int bwfs_save_image(const char *path, bwfs_superblock *sb);

// Carga el superbloque desde un archivo binario
int bwfs_load_image(const char *path, bwfs_superblock *sb);

#endif
