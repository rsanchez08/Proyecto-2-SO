#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"
#include "errno.h"

int create_empty_image(const char* path) {
    uint8_t *pixels = calloc(BLOCK_PIXELS, 1);
    if (!pixels) return -1;
    
    int result = stbi_write_png(path, BLOCK_WIDTH, BLOCK_HEIGHT, 
                              1, pixels, BLOCK_WIDTH);
    free(pixels);
    return result ? 0 : -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: mkfs.bwfs <carpeta_destino> [tamaño_MB]\n");
        return 1;
    }

    const char *carpeta = argv[1];
    uint32_t size_mb = (argc == 3) ? atoi(argv[2]) : 100;

    // Validar tamaño mínimo
    if (size_mb < 1) {
        fprintf(stderr, "Error: Tamaño mínimo es 1MB\n");
        return 1;
    }

    // Calcular bloques necesarios 
    uint32_t total_blocks = (size_mb * 1024 * 1024) / BWFS_BLOCK_SIZE;
    
    // Validar tamaño máximo
    if (total_blocks > BWFS_MAX_BLOCKS) {
        fprintf(stderr, "Error: Tamaño máximo excedido (máx %lu MB)\n", 
       ((unsigned long)BWFS_MAX_BLOCKS * BWFS_BLOCK_SIZE) / (1024 * 1024));
        return 1;
    }

    // Crear carpeta
    if (mkdir(carpeta, 0777) && errno != EEXIST) {
        perror("Error al crear directorio");
        return 1;
    }

    // Calcular tamaño de metadatos
    size_t bitmap_size = (total_blocks + 7) / 8;
    size_t sb_size = sizeof(bwfs_superblock) + bitmap_size; // Superbloque + bitmap
    size_t inode_table_size = sizeof(bwfs_inode) * BWFS_MAX_FILES;

    // Verificar que el superbloque + bitmap quepa en un bloque
    if (sizeof(bwfs_superblock) + bitmap_size > BWFS_BLOCK_SIZE) {
        fprintf(stderr, "Error: El superbloque excede el tamaño de bloque\n");
        return 1;
    }

    // Calcular bloques necesarios 
    size_t total_metadata_size = sb_size + inode_table_size;
    uint32_t metadata_blocks = (total_metadata_size + BWFS_BLOCK_SIZE - 1) / BWFS_BLOCK_SIZE;
    
    // Asignar y preparar buffers
    bwfs_superblock *sb = malloc(sb_size);
    void *inode_table = calloc(1, inode_table_size);
    uint8_t *metadata_buffer = malloc(metadata_blocks * BWFS_BLOCK_SIZE);
    
    // Inicializar estructuras
    memset(sb, 0, sb_size);
    sb->magic = BWFS_MAGIC;
    sb->block_size = BWFS_BLOCK_SIZE;
    sb->total_blocks = total_blocks;
    sb->free_blocks = total_blocks - metadata_blocks; // Reservar bloques de metadatos
    sb->total_inodes = BWFS_MAX_FILES;
    sb->free_inodes = BWFS_MAX_FILES;
    sb->blocks_per_image = 1;
    sb->first_data_block = metadata_blocks; // Los datos empiezan después de los metadatos
    sb->metadata_block_count = metadata_blocks; // Nuevo campo necesario en el superbloque
    
    // Inicializar bitmap
    uint8_t* bitmap = (uint8_t*)(sb + 1);
    memset(bitmap, 0, bitmap_size);
    for (uint32_t i = 0; i < metadata_blocks; i++) {
        bitmap[i/8] |= (1 << (i%8)); // Marcar bloques de metadatos como usados
    }

    // Copiar todo al buffer de metadatos
    memcpy(metadata_buffer, sb, sb_size);
    memcpy(metadata_buffer + sb_size, inode_table, inode_table_size);
    
    // Guardar bloques de metadatos
    for (uint32_t i = 0; i < metadata_blocks; i++) {
        char meta_path[512];
        snprintf(meta_path, sizeof(meta_path), "%s/%06d_meta.png", carpeta, i);
        bwfs_save_image(meta_path, metadata_buffer + i*BWFS_BLOCK_SIZE, BWFS_BLOCK_SIZE);
    }

    // Crear bloques de datos
    for (uint32_t i = metadata_blocks; i < total_blocks; i++) {
        char block_path[512];
        snprintf(block_path, sizeof(block_path), "%s/%06d_data.png", carpeta, i);
        create_empty_image(block_path);
    }

    // Liberar recursos
    free(metadata_buffer);
    free(inode_table);
    free(sb);

    printf("Sistema de archivos BWFS creado exitosamente en: %s\n", carpeta);
    printf("- Tamaño configurado: %d MB (%d bloques de 4KB)\n", size_mb, total_blocks);
    printf("- Bloques de datos creados: %d\n", total_blocks);
    printf("- Archivo de metadatos: 000000_meta.png\n");
    printf("- Tabla de inodos: 000001_meta.png - 000005_meta.png\n");
    
    return 0;
}