#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

// Puntero global al superbloque cargado
bwfs_superblock *sb = NULL;

// Arreglo global de inodos
bwfs_inode inodes[BWFS_MAX_FILES];

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: fsck.bwfs <carpeta_sistema>\n");
        return 1;
    }

    const char *carpeta = argv[1];

    // Rutas de archivos
    char path_meta[512], path_inodes[512];
    snprintf(path_meta, sizeof(path_meta), "%s/meta.bwfs", carpeta);
    snprintf(path_inodes, sizeof(path_inodes), "%s/inodes.bwfs", carpeta);

    // Cargar superbloque + bitmap
    if (bwfs_load_image(path_meta, &sb) != 0) {
        fprintf(stderr, "❌ Error: No se pudo leer %s\n", path_meta);
        return 1;
    }

    // Validar magic
    if (sb->magic != BWFS_MAGIC) {
        fprintf(stderr, "❌ Error: Magic inválido (esperado 0x%X, encontrado 0x%X)\n",
                BWFS_MAGIC, sb->magic);
        return 1;
    }

    // Cargar tabla de inodos
    FILE *fin = fopen(path_inodes, "rb");
    if (!fin) {
        perror("❌ Error al abrir inodes.bwfs");
        return 1;
    }
    fread(inodes, sizeof(bwfs_inode), BWFS_MAX_FILES, fin);
    fclose(fin);

    // Preparar para verificación del bitmap
    size_t bitmap_size = (sb->total_blocks + 7) / 8;
    unsigned char bitmap_reconstruido[bitmap_size];
    memset(bitmap_reconstruido, 0, bitmap_size);

    // Marcar bloque 0 como reservado (igual que mkfs)
    bitmap_reconstruido[0] = 0x01;

    int archivos = 0;
    int inconsistencias = 0;

    // Reconstruir el bitmap a partir de los inodos válidos
    for (int i = 0; i < BWFS_MAX_FILES; i++) {
        if (inodes[i].used) {
            archivos++;

            for (int j = 0; j < BWFS_BLOCKS_PER_INODE; j++) {
                uint32_t blk = inodes[i].blocks[j];

                // Validar que el bloque es válido
                if (blk > 0 && blk < sb->total_blocks) {
                    bitmap_reconstruido[blk / 8] |= (1 << (blk % 8));
                }
            }
        }
    }

    // Comparar bitmaps
    for (size_t i = 0; i < bitmap_size; i++) {
        if (sb->bitmap[i] != bitmap_reconstruido[i]) {
            printf("⚠️  Diferencia en bitmap[%zu]: esperado 0x%02X, actual 0x%02X\n",
                   i, bitmap_reconstruido[i], sb->bitmap[i]);
            inconsistencias++;
        }
    }

    // Resumen
    printf("\n📋 Resultado de verificación:\n");
    printf("Archivos encontrados:     %d\n", archivos);
    printf("Total de bloques:         %u\n", sb->total_blocks);
    printf("Tamaño del bitmap:        %zu bytes\n", bitmap_size);
    printf("Inconsistencias detectadas: %d\n", inconsistencias);
    printf("Estado del sistema:       %s\n", (inconsistencias == 0) ? "✅ CONSISTENTE" : "❌ INCONSISTENTE");

    // Limpieza
    free(sb);
    return (inconsistencias == 0) ? 0 : 2;
}
