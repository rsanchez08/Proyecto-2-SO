/*
mkfs.bwfs - Creador del sistema de archivos BWFS.

1. Inicializa la estructura del superbloque con parámetros definidos en bwfs.h.
2. Llena el bitmap de bloques con ceros.
3. Guarda el resultado como "FS_0.png" en la carpeta especificada (imagen simulada del disco).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: mkfs.bwfs <ruta_salida>\n");
        return 1;
    }

    const char *output_folder = argv[1];
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/FS_0.png", output_folder);

    // Crear carpeta si no existe
    mkdir(output_folder, 0777);

    // Inicializar estructura
    bwfs_superblock sb;
    memset(&sb, 0, sizeof(sb));
    sb.magic = BWFS_MAGIC;
    sb.num_inodes = BWFS_MAX_FILES;
    sb.num_blocks = BWFS_MAX_BLOCKS;

    // Guardar imagen inicial
    if (bwfs_save_image(output_path, &sb) != 0) {
        fprintf(stderr, "Error al guardar la imagen FS en %s\n", output_path);
        return 1;
    }

    printf("Sistema de archivos BWFS creado en: %s\n", output_path);
    return 0;
}
