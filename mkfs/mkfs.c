#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"
#include "../mkfs/stb_image.h"
#include "../mkfs/stb_image_write.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: mkfs.bwfs <carpeta_destino>\n");
        return 1;
    }

    // Crear la carpeta si no existe
    mkdir(argv[1], 0777);

    // Construir ruta a FS_0.png
    char path[512];
    snprintf(path, sizeof(path), "%s/FS_0.png", argv[1]);

    // Inicializar el superbloque
    bwfs_superblock sb;
    memset(&sb, 0, sizeof(sb));
    sb.magic = BWFS_MAGIC;
    sb.num_inodes = BWFS_MAX_FILES;
    sb.num_blocks = BWFS_MAX_BLOCKS;

    // Guardar el FS_0.png como archivo binario
    if (bwfs_save_image(path, &sb) != 0) {
        fprintf(stderr, "Error creando el FS en %s\n", path);
        return 1;
    }

    printf("Sistema de archivos BWFS creado en: %s\n", path);
    return 0;
}
