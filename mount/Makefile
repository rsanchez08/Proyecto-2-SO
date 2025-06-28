# Makefile para compilar el montador BWFS con FUSE y stb

CC = gcc
CFLAGS = -Wall -Wextra -I../bwfs -I../mkfs
SRC = mount.c bwfs.c ../bwfs/bwfs_io.c
OUT = mount

# `pkg-config fuse3 --cflags --libs` busca las rutas y librerías de FUSE
# `-lm` es para funciones matemáticas usadas por stb_image

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) `pkg-config fuse3 --cflags --libs` -lm

clean:
	rm -f $(OUT)
