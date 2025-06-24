#include <stdio.h>
#include <string.h>
#include "bwfs_io.h"

// Guarda el superbloque en un archivo simulado (puede ser imagen PNG en futuras versiones)
int bwfs_save_image(const char *path, bwfs_superblock *sb) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;
    fwrite(sb, sizeof(bwfs_superblock), 1, fp);
    fclose(fp);
    return 0;
}

// Carga el superbloque desde un archivo simulado
int bwfs_load_image(const char *path, bwfs_superblock *sb) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    fread(sb, sizeof(bwfs_superblock), 1, fp);
    fclose(fp);
    return 0;
}
