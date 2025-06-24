# Makefile para mkfs.bwfs

CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -I../bwfs

all: mkfs

mkfs: mkfs.c
	$(CC) $(CFLAGS) mkfs.c ../bwfs/bwfs_io.c -o mkfs $(INCLUDES)

clean:
	rm -f mkfs
