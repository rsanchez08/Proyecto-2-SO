# Makefile para compilar mkfs con soporte para PNG usando stb_image_write

CC = gcc
CFLAGS = -Wall -Wextra -I../bwfs

SRC = mkfs.c ../bwfs/bwfs_io.c

all: mkfs

mkfs: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o mkfs

clean:
	rm -f mkfs
