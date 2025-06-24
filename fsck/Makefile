# Makefile para fsck.bwfs

CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -I../bwfs

all: fsck

fsck: fsck.c
	$(CC) $(CFLAGS) fsck.c ../bwfs/bwfs_io.c -o fsck $(INCLUDES)

clean:
	rm -f fsck
