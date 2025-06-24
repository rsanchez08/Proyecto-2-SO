# Makefile para mount.bwfs con FUSE

CC = gcc
CFLAGS = -Wall -Wextra
FUSE_FLAGS = `pkg-config fuse3 --cflags --libs`
INCLUDES = -I../bwfs

all: mount

mount: mount.c bwfs_ops.c
	$(CC) $(CFLAGS) mount.c bwfs_ops.c ../bwfs/bwfs_io.c -o mount $(INCLUDES) $(FUSE_FLAGS)

clean:
	rm -f mount
