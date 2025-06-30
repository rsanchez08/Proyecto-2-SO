#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: fsck <carpeta_fs>\n");
        return 1;
    }

    const char *carpeta = argv[1];
    char path[512];
    snprintf(path, sizeof(path), "%s/000000_meta.png", carpeta);

    void *meta_block = NULL;
    size_t meta_size = 0;

    // Intentar cargar el archivo del superbloque
    if (bwfs_load_image(path, &meta_block, &meta_size) != 0) {
        fprintf(stderr, "No se pudo cargar el archivo: %s\n", path);
        return 1;
    }

    bwfs_superblock *sb = (bwfs_superblock *)meta_block;

    // Validar magic
    if (sb->magic != BWFS_MAGIC) {
        fprintf(stderr, "Magic inválido: no es un sistema de archivos BWFS\n");
        free(meta_block);
        return 1;
    }

    // Validaciones 
    if (sb->block_size != BWFS_BLOCK_SIZE)
        printf("Tamaño de bloque inesperado: %u (esperado %d)\n", sb->block_size, BWFS_BLOCK_SIZE);

    if (sb->total_blocks == 0)
        printf("total_blocks es 0\n");

    if (sb->first_data_block < sb->metadata_block_count)
        printf("first_data_block (%u) es menor que metadata_block_count (%u)\n",
               sb->first_data_block, sb->metadata_block_count);

    if (sb->free_blocks > sb->total_blocks)
        printf("free_blocks fuera de rango\n");

    if (sb->free_inodes > sb->total_inodes)
        printf("free_inodes fuera de rango\n");

    // Imprimir info del superbloque
    printf("Superbloque válido\n");
    printf("Total bloques:        %u\n", sb->total_blocks);
    printf("Bloques libres:       %u\n", sb->free_blocks);
    printf("Inodos totales:       %u\n", sb->total_inodes);
    printf("Inodos libres:        %u\n", sb->free_inodes);
    printf("Bloques por imagen:   %u\n", sb->blocks_per_image);
    printf("Bloques de metadatos: %u\n", sb->metadata_block_count);
    printf("Primer bloque de datos: %u\n", sb->first_data_block);

    free(meta_block);
    return 0;
}
