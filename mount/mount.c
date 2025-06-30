#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

// Variables globales compartidas (declaradas como extern en bwfs.c)
extern bwfs_superblock *sb;
extern bwfs_inode inodes[BWFS_MAX_FILES];
extern char blocks_folder[512];
extern struct fuse_operations bwfs_oper;

// Función para cargar superbloque, bitmap e inodos desde imágenes *_meta.png
int cargar_metadata(const char *carpeta) {
    // 1. Leer el primer bloque meta para obtener la estructura del superbloque
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%06d_meta.png", carpeta, 0);

    void *first_block = NULL;
    size_t block_size = 0;
    if (bwfs_load_image(path, &first_block, &block_size) != 0) {
        fprintf(stderr, "No se pudo leer %s\n", path);
        return -1;
    }

    // Copiar temporalmente el superbloque
    bwfs_superblock temp_sb;
    memcpy(&temp_sb, first_block, sizeof(bwfs_superblock));

    // Calcular tamaños reales
    size_t real_bitmap_size = (temp_sb.total_blocks + 7) / 8;
    size_t real_sb_size = sizeof(bwfs_superblock) + real_bitmap_size;
    size_t inode_table_size = sizeof(bwfs_inode) * temp_sb.total_inodes;
    size_t total_metadata_size = real_sb_size + inode_table_size;
    uint32_t metadata_blocks = (total_metadata_size + BWFS_BLOCK_SIZE - 1) / BWFS_BLOCK_SIZE;

    // Leer todos los bloques meta necesarios
    uint8_t *metadata = malloc(metadata_blocks * BWFS_BLOCK_SIZE);
    for (uint32_t i = 0; i < metadata_blocks; i++) {
        snprintf(path, sizeof(path), "%s/%06d_meta.png", carpeta, i);
        void *block = NULL;
        size_t size = 0;
        if (bwfs_load_image(path, &block, &size) != 0) {
            fprintf(stderr, "Faltó %s\n", path);
            free(metadata);
            return -1;
        }
        memcpy(metadata + i * BWFS_BLOCK_SIZE, block, BWFS_BLOCK_SIZE);
        free(block);
    }

    // Asignar superbloque e inodos desde el buffer cargado
    sb = (bwfs_superblock *)malloc(real_sb_size);
    memcpy(sb, metadata, real_sb_size);
    memcpy(inodes, metadata + real_sb_size, inode_table_size);

    free(metadata);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: mount.bwfs <carpeta_del_FS> <punto_de_montaje>\n");
        return 1;
    }

    // Verificar que la carpeta sea válida
    struct stat st;
    if (stat(argv[1], &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Carpeta inválida: %s\n", argv[1]);
        return 1;
    }

    // Obtener ruta absoluta
    char resolved_path[PATH_MAX];
    if (!realpath(argv[1], resolved_path)) {
        perror("realpath");
        return 1;
    }

    strncpy(blocks_folder, resolved_path, sizeof(blocks_folder));

    // Cargar metadata desde las imágenes
    if (cargar_metadata(blocks_folder) != 0) {
        return 1;
    }

    // Ejecutar FUSE en modo foreground
    char *fuse_argv[] = { argv[0], argv[2], "-f" };
    return fuse_main(3, fuse_argv, &bwfs_oper, NULL);
}
