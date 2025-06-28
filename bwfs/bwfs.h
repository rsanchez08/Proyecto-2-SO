#ifndef BWFS_H
#define BWFS_H

#include <time.h>
#include <sys/types.h>

#define BWFS_MAGIC 0xBEEFBEEF         // Firma mágica del sistema de archivos
#define BWFS_BLOCK_SIZE 4096          // Tamaño simulado de bloque (en bytes)
#define BWFS_MAX_BLOCKS 1024          // Número máximo de bloques disponibles
#define BWFS_MAX_FILES 128            // Número máximo de archivos (inodos)
#define BWFS_FILENAME_MAX 255         // Longitud máxima de nombre de archivo

// Estructura para representar un archivo (inode)
typedef struct {
    int used;                         // 1 si está usado, 0 si está libre
    char filename[BWFS_FILENAME_MAX]; // Nombre del archivo
    int size;                         // Tamaño del archivo en bytes
    int blocks[10];                   // Reservado para uso futuro (fragmentación)
    mode_t mode;                      // Permisos POSIX (lectura, escritura, etc.)
    time_t atime, mtime, ctime;       // Tiempos de acceso, modificación y creación
} bwfs_inode;

// Superbloque que contiene toda la metadata del FS
typedef struct {
    int magic;                                      // Firma mágica del FS
    int num_inodes;                                // Número de inodos disponibles
    int num_blocks;                                // Número de bloques disponibles
    unsigned char block_bitmap[BWFS_MAX_BLOCKS];   // Mapa de bloques usados/libres
    bwfs_inode inodes[BWFS_MAX_FILES];             // Tabla de inodos (archivos)
} bwfs_superblock;

#endif
