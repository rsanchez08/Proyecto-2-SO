#include <stdio.h>
#include <string.h>
#include "bwfs_io.h"

/*
 * Guarda el contenido del superbloque en un archivo (ej. FS_0.png).
 * Retorna 0 si tiene éxito, -1 si hay error.
 */
int bwfs_save_image(const char *path, bwfs_superblock *sb) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;

    fwrite(sb, sizeof(bwfs_superblock), 1, fp);
    fclose(fp);
    return 0;
}

/*
 * Carga el contenido de un archivo (ej. FS_0.png) al superbloque.
 * Retorna 0 si tiene éxito, -1 si hay error.
 */
int bwfs_load_image(const char *path, bwfs_superblock *sb) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;

    fread(sb, sizeof(bwfs_superblock), 1, fp);
    fclose(fp);
    return 0;
}