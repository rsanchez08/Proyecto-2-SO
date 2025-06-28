#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"
#include "../mkfs/stb_image.h"
#include "../mkfs/stb_image_write.h"


// Variables globales (declaradas en bwfs.c)
extern bwfs_superblock sb;
extern char fs_path[512];
extern char blocks_folder[512];
extern struct fuse_operations bwfs_oper;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: mount.bwfs <carpeta_con_FS_0.png> <punto_de_montaje>\n");
        return 1;
    }

    // Verificar que la carpeta de bloques existe
    struct stat st = {0};
    if (stat(argv[1], &st) == -1 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "❌ Error: la carpeta de bloques no existe o no es un directorio: %s\n", argv[1]);
        return 1;
    }

    // Obtener ruta absoluta
    char resolved_path[PATH_MAX];
    if (!realpath(argv[1], resolved_path)) {
        perror("realpath");
        return 1;
    }

    // Asignar ruta global
    strncpy(blocks_folder, resolved_path, sizeof(blocks_folder));
    blocks_folder[sizeof(blocks_folder) - 1] = '\0';

    // Construir ruta a FS_0.png
    snprintf(fs_path, sizeof(fs_path), "%s/FS_0.png", blocks_folder);

    // Cargar el superbloque binario
    if (bwfs_load_image(fs_path, &sb) != 0) {
        fprintf(stderr, "❌ Error al abrir %s\n", fs_path);
        return 1;
    }

    // Verificar magic
    if (sb.magic != BWFS_MAGIC) {
        fprintf(stderr, "❌ Error: sistema de archivos no válido (firma incorrecta)\n");
        return 1;
    }

    // Lanzar FUSE
    char *fuse_argv[] = { argv[0], argv[2], "-f" };
    return fuse_main(3, fuse_argv, &bwfs_oper, NULL);
}
