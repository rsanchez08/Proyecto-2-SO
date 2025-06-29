#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bwfs_io.h"
#include <stdint.h>
#include <stddef.h> // Para offsetof()

/*
 * Guarda el superbloque y su bitmap en un archivo.
 * Retorna 0 si tiene éxito, -1 si hay error.
 */
int bwfs_save_image(const char *path, const bwfs_superblock *sb) {
    if (!path || !sb || sb->magic != BWFS_MAGIC) {
        return -1;
    }

    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;

    // 1. Escribir parte fija del superbloque
    size_t fixed_size = offsetof(bwfs_superblock, bitmap);
    if (fwrite(sb, 1, fixed_size, fp) != fixed_size) {
        fclose(fp);
        return -1;
    }

    // 2. Escribir bitmap (ubicado justo después del superbloque en memoria)
    size_t bitmap_size = (sb->total_blocks + 7) / 8;
    const uint8_t* bitmap = (const uint8_t*)(sb + 1);
    
    if (fwrite(bitmap, 1, bitmap_size, fp) != bitmap_size) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

/*
 * Carga el superbloque y bitmap desde un archivo.
 * Retorna 0 si tiene éxito, -1 si hay error.
 * Nota: El caller debe liberar la memoria con free().
 */
int bwfs_load_image(const char *path, bwfs_superblock **sb_ptr) {
    if (!path || !sb_ptr) return -1;

    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;

    // 1. Leer parte fija para obtener total_blocks
    bwfs_superblock sb_temp;
    if (fread(&sb_temp, 1, offsetof(bwfs_superblock, bitmap), fp) != offsetof(bwfs_superblock, bitmap)) {
        fclose(fp);
        return -1;
    }

    // Validar magic number
    if (sb_temp.magic != BWFS_MAGIC) {
        fclose(fp);
        return -1;
    }

    // 2. Asignar memoria completa (superbloque + bitmap)
    size_t bitmap_size = (sb_temp.total_blocks + 7) / 8;
    size_t total_size = offsetof(bwfs_superblock, bitmap) + bitmap_size;
    bwfs_superblock *sb = malloc(total_size);
    if (!sb) {
        fclose(fp);
        return -1;
    }

    // 3. Copiar parte fija ya leída
    memcpy(sb, &sb_temp, offsetof(bwfs_superblock, bitmap));

    // 4. Leer bitmap
    if (fread((uint8_t*)sb + offsetof(bwfs_superblock, bitmap), 1, bitmap_size, fp) != bitmap_size) {
        free(sb);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    *sb_ptr = sb;
    return 0;
}