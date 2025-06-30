# Makefile para compilar el montador BWFS con FUSE y stb_image

CC = gcc
CFLAGS = -Wall -Wextra -I../bwfs -I../mkfs
SRC = mount.c bwfs.c ../bwfs/bwfs_io.c
OUT = mount.bwfs

# Comando para obtener flags de FUSE 3
FUSE_FLAGS = $(shell pkg-config fuse3 --cflags --libs)

# `-lm` necesario para stb_image (funciones matemáticas internas)
all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(FUSE_FLAGS) -lm

clean:
	rm -f $(OUT)
