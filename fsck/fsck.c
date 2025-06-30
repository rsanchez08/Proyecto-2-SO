#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

// Rango de bloques que contienen inodos (bloques de metadatos del 1 al 6)
#define META_INODE_START 1
#define META_INODE_END 6

int main(int argc, char *argv[]) {
    // Verifica argumentos de entrada
    if (argc != 2) {
        fprintf(stderr, "Uso: fsck <carpeta_fs>\n");
        return 1;
    }

    char path[512];
    const char *carpeta = argv[1];

    // Cargar el superbloque desde el archivo 000000_meta.png
    snprintf(path, sizeof(path), "%s/000000_meta.png", carpeta);
    void *meta_block = NULL;
    size_t meta_size = 0;
    if (bwfs_load_image(path, &meta_block, &meta_size) != 0) {
        fprintf(stderr, "Error al cargar el archivo: %s\n", path);
        return 1;
    }

    // Interpretar la estructura del superbloque
    bwfs_superblock *sb = (bwfs_superblock *)meta_block;
    if (sb->magic != BWFS_MAGIC) {
        fprintf(stderr, "Firma del FS inválida\n");
        free(meta_block);
        return 1;
    }

    // El bitmap se encuentra justo después del superbloque
    uint8_t *bitmap = (uint8_t *)(sb + 1);

    // Mostrar información básica del superbloque
    printf("Superbloque válido\n");
    printf("Total bloques: %u\n", sb->total_blocks);
    printf("Bloques libres: %u\n", sb->free_blocks);
    printf("Inodos totales: %u\n", sb->total_inodes);
    printf("Inodos libres: %u\n", sb->free_inodes);
    printf("Bloques por imagen: %u\n", sb->blocks_per_image);
    printf("Bloques de metadatos: %u\n", sb->metadata_block_count);

    // Cargar todos los bloques que contienen la tabla de inodos
    bwfs_inode inodes[BWFS_MAX_FILES];
    memset(inodes, 0, sizeof(inodes));  // Inicializar inodos a cero
    int inode_idx = 0;

    for (int i = META_INODE_START; i <= META_INODE_END; i++) {
        snprintf(path, sizeof(path), "%s/%06d_meta.png", carpeta, i);
        void *inode_block = NULL;
        size_t inode_size = 0;

        // Cargar bloque de inodos individual
        if (bwfs_load_image(path, &inode_block, &inode_size) != 0) {
            fprintf(stderr, "Error al cargar inodos desde %s\n", path);
            free(meta_block);
            return 1;
        }

        // Calcular cuántos inodos caben en el bloque cargado
        size_t count = inode_size / sizeof(bwfs_inode);
        if (inode_idx + count > BWFS_MAX_FILES)
            count = BWFS_MAX_FILES - inode_idx;

        // Copiar los inodos al arreglo principal
        memcpy(&inodes[inode_idx], inode_block, count * sizeof(bwfs_inode));
        inode_idx += count;
        free(inode_block);
    }

    // Validar consistencia de los bloques usados por los inodos
    int errors = 0;
    uint8_t *block_usage = calloc(sb->total_blocks, 1); // Vector de uso real de bloques
    if (!block_usage) {
        fprintf(stderr, "No hay memoria para block_usage\n");
        free(meta_block);
        return 1;
    }

    for (int i = 0; i < BWFS_MAX_FILES; i++) {
        if (!inodes[i].used) continue; // Saltar inodos no usados

        // Validar todos los bloques referenciados por el inodo
        for (uint32_t j = 0; j < inodes[i].block_count && j < BWFS_BLOCKS_PER_INODE; j++) {
            uint32_t b = inodes[i].blocks[j];

            // Verificar límites válidos del bloque
            if (b == 0 || b >= sb->total_blocks || b < sb->first_data_block) {
                printf("Inodo %d usa bloque inválido %u\n", i, b);
                errors++;
                continue;
            }

            // Verificar que el bloque no esté duplicado
            if (block_usage[b]) {
                printf("Bloque %u está duplicado (ya usado)\n", b);
                errors++;
            }

            block_usage[b] = 1; // Marcar bloque como usado
        }
    }

    // Verificar que el bitmap coincida con el uso real
    for (uint32_t b = sb->first_data_block; b < sb->total_blocks; b++) {
        int bit = (bitmap[b / 8] >> (b % 8)) & 1;
        if (bit != block_usage[b]) {
            printf("Desajuste bitmap: bloque %u → bitmap=%d, uso real=%d\n", b, bit, block_usage[b]);
            errors++;
        }
    }

    // Mostrar resumen 
    if (errors == 0)
        printf("FS consistente\n");
    else
        printf("Se encontraron %d errores en el FS\n", errors);

    // Liberar recursos usados
    free(block_usage);
    free(meta_block);
    return 0;
}
