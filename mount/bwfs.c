#define FUSE_USE_VERSION 31

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../mkfs/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../mkfs/stb_image.h"

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

bwfs_superblock sb;
char fs_path[512];
char blocks_folder[512];

// Guarda datos en FS_#.png
int save_block_png(int index, const char *data, size_t size) {
    char path[512];
    snprintf(path, sizeof(path), "%s/FS_%d.png", blocks_folder, index);

    // Depuración
    printf("📁 DEBUG: blocks_folder = '%s'\n", blocks_folder);
    printf("📁 DEBUG: guardando en  = '%s'\n", path);

    // Validar carpeta
    struct stat st = {0};
    if (stat(blocks_folder, &st) == -1 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "❌ Carpeta de bloques no encontrada: %s\n", blocks_folder);
        return 0;
    }

    unsigned char pixels[BLOCK_PIXELS];
    memset(pixels, 0, sizeof(pixels));

    for (size_t i = 0; i < size && i < BLOCK_PIXELS; ++i)
        pixels[i] = data[i] ? 255 : 0;

    int res = stbi_write_png(path, BLOCK_WIDTH, BLOCK_HEIGHT, 1, pixels, BLOCK_WIDTH);
    if (!res) {
        fprintf(stderr, "❌ Error guardando imagen en %s\n", path);
        return 0;
    }

    printf("✅ Bloque guardado: %s (bytes: %zu)\n", path, size);
    return 1;
}

// Lee datos de FS_#.png
int load_block_png(int index, char *buf, size_t size, off_t offset) {
    char path[512];
    snprintf(path, sizeof(path), "%s/FS_%d.png", blocks_folder, index);

    int w, h, ch;
    unsigned char *pixels = stbi_load(path, &w, &h, &ch, 1);
    if (!pixels || w != BLOCK_WIDTH || h != BLOCK_HEIGHT) {
        fprintf(stderr, "❌ No se pudo leer imagen %s\n", path);
        if (pixels) stbi_image_free(pixels);
        return -1;
    }

    size_t total = w * h;
    if ((size_t)offset >= total) {
        stbi_image_free(pixels);
        return 0;
    }

    size_t to_read = (offset + size > total) ? (total - offset) : size;
    for (size_t i = 0; i < to_read; ++i)
        buf[i] = pixels[offset + i] > 128 ? '1' : 0;

    stbi_image_free(pixels);
    return to_read;
}

// FUSE: getattr
static int bwfs_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) {
    (void) fi;
    memset(st, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
        return 0;
    }

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0) {
            st->st_mode = sb.inodes[i].mode;
            st->st_nlink = 1;
            st->st_size = sb.inodes[i].size;
            st->st_ctime = sb.inodes[i].ctime;
            st->st_mtime = sb.inodes[i].mtime;
            st->st_atime = sb.inodes[i].atime;
            return 0;
        }
    }

    return -ENOENT;
}

// FUSE: create
static int bwfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) fi;
    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (!sb.inodes[i].used) {
            sb.inodes[i].used = 1;
            strncpy(sb.inodes[i].filename, path + 1, BWFS_FILENAME_MAX);
            sb.inodes[i].mode = S_IFREG | mode;
            sb.inodes[i].ctime = sb.inodes[i].mtime = sb.inodes[i].atime = time(NULL);
            sb.inodes[i].size = 0;
            sb.inodes[i].blocks[0] = i + 1;  // evita FS_0.png
            bwfs_save_image(fs_path, &sb);
            printf("🆕 Archivo creado: %s → FS_%d.png\n", sb.inodes[i].filename, sb.inodes[i].blocks[0]);
            return 0;
        }
    }
    return -ENOSPC;
}

// FUSE: read
static int bwfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0) {
            return load_block_png(sb.inodes[i].blocks[0], buf, size, offset);
        }
    }
    return -ENOENT;
}

// FUSE: write
static int bwfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0) {
            sb.inodes[i].size = offset + size;
            sb.inodes[i].mtime = time(NULL);
            if (!save_block_png(sb.inodes[i].blocks[0], buf, size)) {
                fprintf(stderr, "❌ Error al guardar FS_%d.png\n", sb.inodes[i].blocks[0]);
                return -EIO;
            }
            bwfs_save_image(fs_path, &sb);
            return size;
        }
    }
    return -ENOENT;
}

// FUSE: readdir
static int bwfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset; (void) fi; (void) flags;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used)
            filler(buf, sb.inodes[i].filename, NULL, 0, 0);
    }

    return 0;
}

// FUSE: open
static int bwfs_open(const char *path, struct fuse_file_info *fi) {
    (void) fi;
    for (int i = 0; i < BWFS_MAX_FILES; ++i)
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0)
            return 0;
    return -ENOENT;
}

// FUSE: unlink
static int bwfs_unlink(const char *path) {
    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0) {
            sb.inodes[i].used = 0;
            sb.inodes[i].size = 0;
            sb.inodes[i].filename[0] = '\0';
            bwfs_save_image(fs_path, &sb);
            printf("🗑️ Archivo eliminado: %s (FS_%d.png)\n", path + 1, sb.inodes[i].blocks[0]);
            return 0;
        }
    }
    return -ENOENT;
}

// FUSE: operaciones registradas
struct fuse_operations bwfs_oper = {
    .getattr = bwfs_getattr,
    .create  = bwfs_create,
    .read    = bwfs_read,
    .write   = bwfs_write,
    .readdir = bwfs_readdir,
    .open    = bwfs_open,
    .unlink  = bwfs_unlink
};
