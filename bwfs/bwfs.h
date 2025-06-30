#ifndef BWFS_H
#define BWFS_H

#include <time.h>
#include <sys/types.h>
#include <stdint.h>

// Identificación del sistema de archivos
#define BWFS_MAGIC 0x42574653 // "BWFS" en ASCII hex

// Límites del sistema
#define BWFS_MAX_FILES 128            // Número máximo de archivos
#define BWFS_FILENAME_MAX 64         // Longitud máxima de nombre
#define BWFS_MAX_BLOCKS (1024 * 1024) // 1M bloques (4TB máximo)

// Configuración de bloques
#define BWFS_BLOCK_SIZE 4096          // 4KB por bloque
#define BWFS_BLOCKS_PER_INODE 16      // Máx 64KB por archivo

// Configuración de imágenes (1 bit por píxel)
#define BLOCK_WIDTH 1000              // Imágenes de 1000 de ancho
#define BLOCK_HEIGHT 1000             // Imágenes de 1000 de alto
#define PIXELS_PER_BYTE 8             // 1 byte = 8 píxeles
#define BLOCK_PIXELS (BLOCK_WIDTH * BLOCK_HEIGHT) // Imágenes de 1000x1000
#define BYTES_PER_IMAGE (BLOCK_PIXELS / PIXELS_PER_BYTE) // 125KB
#define BLOCKS_PER_IMAGE (BYTES_PER_IMAGE / BWFS_BLOCK_SIZE) // 30 bloques

// Estructura de inodo
typedef struct {
    uint8_t used;                    // 1=usado, 0=libre
    char filename[BWFS_FILENAME_MAX]; // Nombre del archivo
    uint32_t size;                   // Tamaño real en bytes
    uint32_t block_count;            // Bloques usados
    uint32_t blocks[BWFS_BLOCKS_PER_INODE]; // Bloques asignados
    mode_t mode;                     // Permisos
    uid_t uid;                       // Usuario
    gid_t gid;                       // Grupo
    time_t atime;                    // Último acceso
    time_t mtime;                    // Última modificación
    time_t ctime;                    // Creación
} bwfs_inode;

// Superbloque (parte fija)
typedef struct {
    uint32_t magic;             // Número mágico para diferenciar el FS
    uint32_t block_size;        // Tamaño de bloques en bytes
    uint32_t total_blocks;      // Bloques totales en el FS
    uint32_t free_blocks;       // Bloques libres
    uint32_t total_inodes;      // Total de inodos
    uint32_t free_inodes;       // Inodos libres
    uint32_t blocks_per_image;  // Bloques por imagen
    uint32_t first_data_block;  // A partir de qué número de bloque terminan los metadatos
    uint8_t bitmap[];           // Mapa de bits
} bwfs_superblock;

#endif