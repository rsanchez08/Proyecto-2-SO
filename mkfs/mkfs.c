/*
mkfs.bwfs - Creador del sistema de archivos BWFS.

Este programa se encarga de:
1. Crear el archivo de imagen `FS_0.png` en la carpeta indicada.
2. Inicializar el superbloque con todos los valores en cero.
3. Guardar el superbloque en la imagen usando bwfs_io.c

Forma de uso:
    ./mkfs <carpeta_destino>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: ./mkfs <carpeta_destino>\n");
        return 1;
    }

    const char *carpeta = argv[1];
    char path[512];
    snprintf(path, sizeof(path), "%s/FS_0.png", carpeta);  // Ruta completa a la imagen FS

    // Crear la carpeta destino si no existe
    mkdir(carpeta, 0777);

    // Inicializar el superbloque con valores por defecto
    bwfs_superblock sb;
    memset(&sb, 0, sizeof(sb));  // llena todo con 0s
    sb.magic = BWFS_MAGIC;
    sb.num_inodes = BWFS_MAX_FILES;
    sb.num_blocks = BWFS_MAX_BLOCKS;

    // Guardar el superbloque en el archivo de imagen
    if (bwfs_save_image(path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo guardar el sistema de archivos en %s\n", path);
        return 1;
    }

    printf("Sistema de archivos creado correctamente.\n");
    printf("Ubicación: %s\n", path);
    printf("Inodos disponibles:     %d\n", sb.num_inodes);
    printf("Bloques disponibles:    %d\n", sb.num_blocks);
    return 0;
}