/*
fsck.bwfs - Verifica la integridad del sistema de archivos BWFS.

1. Carga el archivo de imagen (simulada o .png más adelante).
2. Verifica que el número mágico BWFS_MAGIC esté presente.
3. Imprime el número de bloques e inodos detectados.

Estructura del proyecto:
- Utiliza funciones de bwfs_io para leer el FS.
- bwfs_superblock está definido en bwfs.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: fsck.bwfs <ruta/carpeta>\n");
        return 1;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/FS_0.png", argv[1]); // simulamos imagen de disco

    bwfs_superblock sb;
    if (bwfs_load_image(path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo cargar el sistema de archivos en %s\n", path);
        return 1;
    }

    if (sb.magic != BWFS_MAGIC) {
        fprintf(stderr, "Error: sistema de archivos no válido (magic incorrecto)\n");
        return 1;
    }

    printf("✓ FS válido\n");
    printf("Archivos posibles: %d\n", sb.num_inodes);
    printf("Bloques disponibles: %d\n", sb.num_blocks);
    return 0;
}
