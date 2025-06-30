#define FUSE_USE_VERSION 31

// Inclusión de bibliotecas STB para manejo de imágenes PNG
#include "../mkfs/stb_image_write.h"
#include "../mkfs/stb_image.h"

// Bibliotecas estándar de FUSE y del sistema
#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

// Variables globales del sistema de archivos
bwfs_superblock *sb;
bwfs_inode inodes[BWFS_MAX_FILES];
char blocks_folder[512];

// Función para guardar el estado completo del sistema de archivos
void bwfs_guardar_estado() {
    size_t bitmap_size = (sb->total_blocks + 7) / 8;
    size_t sb_size = sizeof(bwfs_superblock) + bitmap_size;
    size_t inode_table_size = sizeof(bwfs_inode) * BWFS_MAX_FILES;
    size_t total_metadata_size = sb_size + inode_table_size;
    uint32_t metadata_blocks = (total_metadata_size + BWFS_BLOCK_SIZE - 1) / BWFS_BLOCK_SIZE;

    uint8_t *metadata_buffer = calloc(metadata_blocks, BWFS_BLOCK_SIZE);
    memcpy(metadata_buffer, sb, sb_size);
    memcpy(metadata_buffer + sb_size, inodes, inode_table_size);

    for (uint32_t i = 0; i < metadata_blocks; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%06d_meta.png", blocks_folder, i);
        bwfs_save_image(path, metadata_buffer + i * BWFS_BLOCK_SIZE, BWFS_BLOCK_SIZE);
    }

    free(metadata_buffer);
}

// Carga el contenido de un bloque desde su archivo PNG
int load_block_png(int block, char *buf, size_t size, off_t offset) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%06d_data.png", blocks_folder, block);

    void *raw = NULL;
    size_t raw_size = 0;
    if (bwfs_load_image(path, &raw, &raw_size) != 0) return -EIO;

    if ((size_t)offset >= raw_size) {
        free(raw);
        return 0;
    }

    size_t to_read = (offset + size > raw_size) ? (raw_size - offset) : size;
    memcpy(buf, (char*)raw + offset, to_read);
    free(raw);
    return to_read;
}

// Guarda los datos de un bloque en un archivo PNG
int save_block_png(int block, const char *buf, size_t size) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%06d_data.png", blocks_folder, block);
    return bwfs_save_image(path, buf, size);
}

// ---------------------- FUNCIONES FUSE ----------------------

// Obtener atributos de archivo o directorio
static int bwfs_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) {
    (void) fi;
    memset(st, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
        return 0;
    }
    for (int i = 0; i < BWFS_MAX_FILES; i++) {
        if (inodes[i].used && strcmp(inodes[i].filename, path + 1) == 0) {
            st->st_mode = inodes[i].mode;
            st->st_size = inodes[i].size;
            st->st_nlink = 1;
            st->st_ctime = inodes[i].ctime;
            st->st_mtime = inodes[i].mtime;
            st->st_atime = inodes[i].atime;
            return 0;
        }
    }
    return -ENOENT;
}

// Leer contenido del directorio
static int bwfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset; (void) fi; (void) flags;
    if (strcmp(path, "/") != 0) return -ENOENT;
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    for (int i = 0; i < BWFS_MAX_FILES; i++)
        if (inodes[i].used)
            filler(buf, inodes[i].filename, NULL, 0, 0);
    return 0;
}

// Crear un archivo
static int bwfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    for (int i = 0; i < BWFS_MAX_FILES; i++) {
        if (!inodes[i].used) {
            int block = -1;
            for (int b = sb->first_data_block; b < sb->total_blocks; b++) {
                if (!(sb->bitmap[b / 8] & (1 << (b % 8)))) {
                    sb->bitmap[b / 8] |= (1 << (b % 8));
                    block = b;
                    break;
                }
            }
            if (block == -1) return -ENOSPC;
            inodes[i].used = 1;
            strncpy(inodes[i].filename, path + 1, BWFS_FILENAME_MAX);
            inodes[i].mode = S_IFREG | mode;
            inodes[i].size = 0;
            inodes[i].blocks[0] = block;
            inodes[i].ctime = inodes[i].mtime = inodes[i].atime = time(NULL);
            bwfs_guardar_estado();
            return 0;
        }
    }
    return -ENOSPC;
}

// Abrir un archivo
static int bwfs_open(const char *path, struct fuse_file_info *fi) {
    for (int i = 0; i < BWFS_MAX_FILES; i++)
        if (inodes[i].used && strcmp(inodes[i].filename, path + 1) == 0)
            return 0;
    return -ENOENT;
}

// Leer datos de un archivo
static int bwfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    for (int i = 0; i < BWFS_MAX_FILES; i++)
        if (inodes[i].used && strcmp(inodes[i].filename, path + 1) == 0)
            return load_block_png(inodes[i].blocks[0], buf, size, offset);
    return -ENOENT;
}

// Escribir datos a un archivo
static int bwfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    for (int i = 0; i < BWFS_MAX_FILES; i++)
        if (inodes[i].used && strcmp(inodes[i].filename, path + 1) == 0) {
            inodes[i].size = offset + size;
            inodes[i].mtime = time(NULL);
            int res = save_block_png(inodes[i].blocks[0], buf, size);
            if (res != 0) return res;
            bwfs_guardar_estado();
            return size;
        }
    return -ENOENT;
}

// Eliminar un archivo
static int bwfs_unlink(const char *path) {
    for (int i = 0; i < BWFS_MAX_FILES; i++) {
        if (inodes[i].used && strcmp(inodes[i].filename, path + 1) == 0) {
            int block = inodes[i].blocks[0];
            sb->bitmap[block / 8] &= ~(1 << (block % 8));

            char ruta[512];
            snprintf(ruta, sizeof(ruta), "%s/%06d_data.png", blocks_folder, block);
            unsigned char negro[BLOCK_PIXELS] = {0};
            stbi_write_png(ruta, BLOCK_WIDTH, BLOCK_HEIGHT, 1, negro, BLOCK_WIDTH);

            memset(&inodes[i], 0, sizeof(bwfs_inode));
            bwfs_guardar_estado();
            return 0;
        }
    }
    return -ENOENT;
}

// Renombrar archivo
static int bwfs_rename(const char *from, const char *to, unsigned int flags) {
    (void) flags;
    for (int i = 0; i < BWFS_MAX_FILES; i++) {
        if (inodes[i].used && strcmp(inodes[i].filename, from + 1) == 0) {
            strncpy(inodes[i].filename, to + 1, BWFS_FILENAME_MAX);
            bwfs_guardar_estado();
            return 0;
        }
    }
    return -ENOENT;
}

// Flush de archivo (guardar estado)
static int bwfs_flush(const char *path, struct fuse_file_info *fi) {
    (void) path; (void) fi;
    bwfs_guardar_estado();
    return 0;
}

// Sincronizar archivo
static int bwfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
    (void) datasync;
    for (int i = 0; i < BWFS_MAX_FILES; i++) {
        if (inodes[i].used && strcmp(inodes[i].filename, path + 1) == 0) {
            inodes[i].mtime = time(NULL);
            bwfs_guardar_estado();
            return 0;
        }
    }
    return -ENOENT;
}

// Información del sistema de archivos
static int bwfs_statfs(const char *path, struct statvfs *stbuf) {
    (void) path;
    memset(stbuf, 0, sizeof(struct statvfs));
    stbuf->f_bsize = BWFS_BLOCK_SIZE;
    stbuf->f_blocks = sb->total_blocks;

    size_t libres = 0;
    for (size_t i = 0; i < sb->total_blocks; ++i)
        if (!(sb->bitmap[i / 8] & (1 << (i % 8))))
            libres++;

    stbuf->f_bfree = libres;
    stbuf->f_bavail = libres;
    stbuf->f_files = BWFS_MAX_FILES;
    stbuf->f_ffree = 0;
    for (int i = 0; i < BWFS_MAX_FILES; i++)
        if (!inodes[i].used)
            stbuf->f_ffree++;
    return 0;
}

// Verificar acceso
static int bwfs_access(const char *path, int mask) {
    (void) mask;
    if (strcmp(path, "/") == 0) return 0;
    for (int i = 0; i < BWFS_MAX_FILES; i++)
        if (inodes[i].used && strcmp(inodes[i].filename, path + 1) == 0)
            return 0;
    return -ENOENT;
}

// Cambiar desplazamiento de lectura/escritura
static off_t bwfs_lseek(const char *path, off_t off, int whence, struct fuse_file_info *fi) {
    (void) path;
    for (int i = 0; i < BWFS_MAX_FILES; i++) {
        if (inodes[i].used && strcmp(inodes[i].filename, path + 1) == 0) {
            off_t size = inodes[i].size;
            off_t new_offset = -1;
            switch (whence) {
                case SEEK_SET: new_offset = off; break;
                case SEEK_CUR: new_offset = fi->fh + off; break;
                case SEEK_END: new_offset = size + off; break;
                default: return -EINVAL;
            }
            if (new_offset < 0 || new_offset > size) return -EINVAL;
            fi->fh = new_offset;
            return new_offset;
        }
    }
    return -ENOENT;
}

// Crear directorio
static int bwfs_mkdir(const char *path, mode_t mode) {
    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (!inodes[i].used) {
            strncpy(inodes[i].filename, path + 1, BWFS_FILENAME_MAX);
            inodes[i].used = 1;
            inodes[i].mode = S_IFDIR | mode;
            inodes[i].size = 0;
            inodes[i].ctime = inodes[i].mtime = inodes[i].atime = time(NULL);
            bwfs_guardar_estado();
            return 0;
        }
    }
    return -ENOSPC;
}

// Tabla de operaciones de FUSE
struct fuse_operations bwfs_oper = {
    .getattr = bwfs_getattr,
    .readdir = bwfs_readdir,
    .create  = bwfs_create,
    .open    = bwfs_open,
    .read    = bwfs_read,
    .write   = bwfs_write,
    .unlink  = bwfs_unlink,
    .rename  = bwfs_rename,
    .flush   = bwfs_flush,
    .fsync   = bwfs_fsync,
    .statfs  = bwfs_statfs,
    .access  = bwfs_access,
    .lseek   = bwfs_lseek,
    .mkdir   = bwfs_mkdir
};
