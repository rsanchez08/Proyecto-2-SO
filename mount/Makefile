# Makefile para compilar mount.bwfs con soporte FUSE

CC = gcc
CFLAGS = -Wall -Wextra
FUSE_FLAGS = `pkg-config fuse3 --cflags --libs`
INCLUDES = -I../bwfs

all: mount

mount: mount.c bwfs.c
	$(CC) $(CFLAGS) mount.c bwfs.c ../bwfs/bwfs_io.c -o mount $(INCLUDES) $(FUSE_FLAGS)

clean:
	rm -f mount
