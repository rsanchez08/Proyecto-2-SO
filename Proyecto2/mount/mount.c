/*
mount.bwfs - Punto de entrada para montar el sistema de archivos BWFS con FUSE.

Uso:
    ./mount <carpeta_con_FS> <punto_de_montaje>

    - <carpeta_con_FS>: Carpeta donde se encuentra la imagen FS_0.png
    - <punto_de_montaje>: Carpeta vacía donde se montará el FS

Este archivo carga el FS, valida el superbloque y lanza FUSE.
*/

#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

// Variables globales accesibles desde bwfs_ops.c
bwfs_superblock sb;
char fs_path[512]; // Ruta completa a FS_0.png

// Operaciones definidas en bwfs_ops.c
extern struct fuse_operations bwfs_oper;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: ./mount <carpeta_con_FS> <punto_de_montaje>\n");
        return 1;
    }

    const char *fs_folder = argv[1];
    const char *mountpoint = argv[2];

    snprintf(fs_path, sizeof(fs_path), "%s/FS_0.png", fs_folder);

    // Cargar el superbloque desde la imagen
    if (bwfs_load_image(fs_path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo abrir la imagen del sistema de archivos: %s\n", fs_path);
        return 1;
    }

    // Validar la firma del sistema de archivos
    if (sb.magic != BWFS_MAGIC) {
        fprintf(stderr, "Error: archivo de sistema de archivos no válido (firma incorrecta)\n");
        return 1;
    }

    printf("✓ FS cargado correctamente desde: %s\n", fs_path);
    printf("→ Montando en: %s\n", mountpoint);

    // Preparar argumentos para FUSE
    char *fuse_argv[] = { argv[0], (char *)mountpoint, "-f" };
    int fuse_argc = 3;

    return fuse_main(fuse_argc, fuse_argv, &bwfs_oper, NULL);
}
