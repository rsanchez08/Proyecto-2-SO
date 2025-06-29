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

    // Calcular imágenes necesarias
    uint32_t images_needed = (total_blocks + BLOCKS_PER_IMAGE - 1) / BLOCKS_PER_IMAGE;
    size_t bitmap_size = (total_blocks + 7) / 8;
    
    // Asignar superbloque + bitmap
    size_t sb_size = sizeof(bwfs_superblock) + bitmap_size;
    bwfs_superblock *sb = malloc(sb_size);
    if (!sb) {
        perror("Error al asignar superbloque");
        return 1;
    }
    memset(sb, 0, sb_size);

    // Inicializar metadatos
    sb->magic = BWFS_MAGIC;
    sb->block_size = BWFS_BLOCK_SIZE;
    sb->total_blocks = total_blocks;
    sb->free_blocks = total_blocks - 1; // Reservar bloque 0
    sb->total_inodes = BWFS_MAX_FILES;
    sb->free_inodes = BWFS_MAX_FILES;
    sb->blocks_per_image = BLOCKS_PER_IMAGE;
    sb->first_data_block = 1;

    // Inicializar bitmap (ubicado justo después del superbloque)
    uint8_t* bitmap = (uint8_t*)(sb + 1);
    memset(bitmap, 0, bitmap_size);
    bitmap[0] = 0x01; // Marcar bloque 0 como usado

    // Guardar metadatos (superbloque + bitmap)
    char meta_path[512];
    snprintf(meta_path, sizeof(meta_path), "%s/meta_0000.png", carpeta);
    if (bwfs_save_image(meta_path, sb, sb_size) != 0) {
        fprintf(stderr, "Error al guardar metadatos\n");
        free(sb);
        return 1;
    }

    // Crear imagen para inodos
    char inodes_path[512];
    snprintf(inodes_path, sizeof(inodes_path), "%s/inodes_0000.png", carpeta);
    size_t inode_table_size = sizeof(bwfs_inode) * BWFS_MAX_FILES;
    void *inode_table = calloc(1, inode_table_size);
    if (!inode_table) {
        fprintf(stderr, "Error al asignar memoria para inodos\n");
        free(sb);
        return 1;
    }
    if (bwfs_save_image(inodes_path, inode_table, inode_table_size) != 0) {
        fprintf(stderr, "Error al guardar inodos como imagen\n");
        free(inode_table);
        free(sb);
        return 1;
    }
    free(inode_table);


    // Crear imágenes de datos
    for (uint32_t i = 0; i < images_needed; i++) {
        char image_path[512];
        snprintf(image_path, sizeof(image_path), "%s/data_%04d.png", carpeta, i+1);
        if (create_empty_image(image_path) != 0) {
            fprintf(stderr, "Error al crear imagen %d\n", i+1);
            free(sb);
            return 1;
        }
    }

    printf("Sistema de archivos BWFS creado exitosamente en: %s\n", carpeta);
    printf("- Tamaño configurado: %d MB (%d bloques de 4KB)\n", size_mb, total_blocks);
    printf("- Imágenes de datos creadas: %d\n", images_needed);
    printf("- Archivo de metadatos: meta_0000.png\n");
    printf("- Tabla de inodos: inodes_0000.png\n");
    
    free(sb);
    return 0;
}