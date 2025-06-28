/*
fsck.bwfs - Verifica la integridad del sistema de archivos BWFS.

1. Carga el archivo de imagen binaria FS_0.png.
2. Verifica que el número mágico BWFS_MAGIC esté presente.
3. Imprime información básica del sistema de archivos.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"
#include "../mkfs/stb_image.h"
#include "../mkfs/stb_image_write.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: fsck.bwfs <ruta/carpeta>\n");
        return 1;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/FS_0.png", argv[1]); // FS_0.png es binario

    bwfs_superblock sb;
    if (bwfs_load_image(path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo abrir %s\n", path);
        return 1;
    }

    if (sb.magic != BWFS_MAGIC) {
        fprintf(stderr, "Error: la imagen no contiene un sistema de archivos válido (firma incorrecta).\n");
        return 1;
    }

    printf("Sistema de archivos BWFS válido.\n");
    printf("Archivos posibles (inodos): %d\n", sb.num_inodes);
    printf("Bloques totales disponibles: %d\n", sb.num_blocks);

    int usados = 0;
    for (int i = 0; i < sb.num_inodes; i++) {
        if (sb.inodes[i].used) usados++;
    }
    printf("Archivos actualmente almacenados: %d\n", usados);

    return 0;
}
