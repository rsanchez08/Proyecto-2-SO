/*
fsck.bwfs - Verifica la integridad del sistema de archivos BWFS.

Este programa se encarga de:
1. Cargar el archivo de imagen del FS desde una carpeta.
2. Validar que el archivo tenga la firma correcta (BWFS_MAGIC).
3. Mostrar cuántos inodos y bloques contiene el FS.

Forma de uso:
    ./fsck <carpeta_con_FS>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: ./fsck <carpeta_con_FS>\n");
        return 1;
    }

    const char *carpeta = argv[1];
    char path[512];
    snprintf(path, sizeof(path), "%s/FS_0.png", carpeta);  // ruta al archivo del FS

    bwfs_superblock sb;

    // Cargar la imagen del sistema de archivos
    if (bwfs_load_image(path, &sb) != 0) {
        fprintf(stderr, "Error: no se pudo abrir la imagen del sistema de archivos en: %s\n", path);
        return 1;
    }

    // Verificar que el número mágico sea correcto
    if (sb.magic != BWFS_MAGIC) {
        fprintf(stderr, "Error: la imagen no contiene un sistema de archivos válido (firma incorrecta).\n");
        return 1;
    }

    // Mostrar información del sistema de archivos
    printf("Sistema de archivos válido.\n");
    printf("Inodos disponibles:     %d\n", sb.num_inodes);
    printf("Bloques disponibles:    %d\n", sb.num_blocks);

    // (Opcional) Puedes agregar más validaciones en el futuro aquí

    return 0;
}