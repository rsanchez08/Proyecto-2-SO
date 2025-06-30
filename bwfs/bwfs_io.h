#ifndef BWFS_IO_H
#define BWFS_IO_H

#include "bwfs.h"

// Guarda el superbloque y la tabla de inodos en una imagen respectivamente
int bwfs_save_image(const char *path, const void *data, size_t size);

// Carga el superbloque y la tabla de inodos en una imagen respectivamente
int bwfs_load_image(const char *path, void **data_ptr, size_t *size_ptr);

#endif // BWFS_IO_H