#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <time.h>
#include "../bwfs/bwfs.h"
#include "../bwfs/bwfs_io.h"

extern bwfs_superblock sb;
extern char fs_path[512];

static int bwfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0) {
            stbuf->st_mode = sb.inodes[i].mode;
            stbuf->st_nlink = 1;
            stbuf->st_size = sb.inodes[i].size;
            stbuf->st_ctime = sb.inodes[i].ctime;
            stbuf->st_mtime = sb.inodes[i].mtime;
            stbuf->st_atime = sb.inodes[i].atime;
            return 0;
        }
    }

    return -ENOENT;
}

static int bwfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) fi;

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (!sb.inodes[i].used) {
            sb.inodes[i].used = 1;
            strncpy(sb.inodes[i].filename, path + 1, BWFS_FILENAME_MAX);
            sb.inodes[i].mode = S_IFREG | mode;
            sb.inodes[i].ctime = sb.inodes[i].mtime = sb.inodes[i].atime = time(NULL);
            sb.inodes[i].size = 0;
            bwfs_save_image(fs_path, &sb);
            return 0;
        }
    }

    return -ENOSPC;
}

static int bwfs_open(const char *path, struct fuse_file_info *fi) {
    (void) fi;

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0)
            return 0;
    }

    return -ENOENT;
}

static int bwfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0) {
            if (offset >= sb.inodes[i].size)
                return 0;
            size_t to_read = size;
            if (offset + size > sb.inodes[i].size)
                to_read = sb.inodes[i].size - offset;
            memset(buf, 0, to_read); // Simula lectura vacía
            return to_read;
        }
    }

    return -ENOENT;
}

static int bwfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) buf;
    (void) fi;

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0) {
            sb.inodes[i].size = offset + size;
            sb.inodes[i].mtime = time(NULL);
            bwfs_save_image(fs_path, &sb);
            return size;
        }
    }

    return -ENOENT;
}

static int bwfs_rename(const char *from, const char *to, unsigned int flags) {
    (void) flags;

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, from + 1) == 0) {
            strncpy(sb.inodes[i].filename, to + 1, BWFS_FILENAME_MAX);
            bwfs_save_image(fs_path, &sb);
            return 0;
        }
    }

    return -ENOENT;
}

static int bwfs_mkdir(const char *path, mode_t mode) {
    (void) path; (void) mode;
    return -EROFS; // Read-only para directorios (no implementado)
}

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

static int bwfs_opendir(const char *path, struct fuse_file_info *fi) {
    (void) fi;
    if (strcmp(path, "/") == 0)
        return 0;
    return -ENOENT;
}

static int bwfs_rmdir(const char *path) {
    (void) path;
    return -EROFS; // No soporte para directorios
}

static int bwfs_statfs(const char *path, struct statvfs *stbuf) {
    (void) path;
    memset(stbuf, 0, sizeof(struct statvfs));
    stbuf->f_bsize = BWFS_BLOCK_SIZE;
    stbuf->f_blocks = BWFS_MAX_BLOCKS;
    stbuf->f_bfree = BWFS_MAX_BLOCKS;
    stbuf->f_bavail = BWFS_MAX_BLOCKS;
    stbuf->f_files = BWFS_MAX_FILES;
    stbuf->f_ffree = BWFS_MAX_FILES;
    return 0;
}

static int bwfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
    (void) path; (void) datasync; (void) fi;
    bwfs_save_image(fs_path, &sb);
    return 0;
}

static int bwfs_access(const char *path, int mask) {
    (void) mask;

    if (strcmp(path, "/") == 0)
        return 0;

    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0)
            return 0;
    }

    return -ENOENT;
}

static int bwfs_unlink(const char *path) {
    for (int i = 0; i < BWFS_MAX_FILES; ++i) {
        if (sb.inodes[i].used && strcmp(sb.inodes[i].filename, path + 1) == 0) {
            sb.inodes[i].used = 0;
            sb.inodes[i].size = 0;
            sb.inodes[i].filename[0] = '\0';
            bwfs_save_image(fs_path, &sb);
            return 0;
        }
    }
    return -ENOENT;
}

static int bwfs_flush(const char *path, struct fuse_file_info *fi) {
    (void) path; (void) fi;
    bwfs_save_image(fs_path, &sb);
    return 0;
}

static off_t bwfs_lseek(const char *path, off_t off, int whence, struct fuse_file_info *fi) {
    (void) path; (void) whence; (void) fi;
    return off;
}

// Registro de todas las operaciones en la estructura FUSE
struct fuse_operations bwfs_oper = {
    .getattr = bwfs_getattr,
    .create  = bwfs_create,
    .open    = bwfs_open,
    .read    = bwfs_read,
    .write   = bwfs_write,
    .rename  = bwfs_rename,
    .mkdir   = bwfs_mkdir,
    .readdir = bwfs_readdir,
    .opendir = bwfs_opendir,
    .rmdir   = bwfs_rmdir,
    .statfs  = bwfs_statfs,
    .fsync   = bwfs_fsync,
    .access  = bwfs_access,
    .unlink  = bwfs_unlink,
    .flush   = bwfs_flush,
    .lseek   = bwfs_lseek
};
