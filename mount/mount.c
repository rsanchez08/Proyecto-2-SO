/*
mount.bwfs - Montador del sistema de archivos BWFS usando FUSE.

Este programa:
1. Carga el archivo de imagen del FS (`FS_0.png`) desde una carpeta.
2. Verifica que sea un sistema válido (chequea el número mágico).
3. Usa FUSE para montar el FS en una carpeta vacía (punto de montaje).

Forma de uso:
    ./mount <carpeta_con_FS> <punto_de_montaje>
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
char fs_path[512];

// Prototipo de operaciones definidas en bwfs_ops.c
extern struct fuse_operations bwfs_oper;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: ./mount <carpeta_con_FS> <punto_de_montaje>\n");
        return 1;
    }

    const char *carpeta_fs = argv[1];
    const char *punto_montaje = argv[2];

    snprintf(fs_path, sizeof(fs_path), "%s/FS_0.png", carpeta_fs);

    // Cargar el superbloque desde la imagen
    if (bwfs_load_image(fs_path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo abrir la imagen en: %s\n", fs_path);
        return 1;
    }

    // Verificar que el sistema sea válido (magic)
    if (sb.magic != BWFS_MAGIC) {
        fprintf(stderr, "Error: sistema de archivos inválido (firma incorrecta)\n");
        return 1;
    }

    printf("FS cargado correctamente desde: %s\n", fs_path);
    printf("Montando en: %s\n", punto_montaje);

    // Preparar argumentos para FUSE
    char *fuse_argv[] = { argv[0], (char *)punto_montaje, "-f" };
    int fuse_argc = 3;

    return fuse_main(fuse_argc, fuse_argv, &bwfs_oper, NULL);
}