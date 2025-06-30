# Makefile para compilar fsck.bwfs que verifica la consistencia del sistema BWFS

CC = gcc
CFLAGS = -Wall -Wextra -I../bwfs -I../mkfs
SRC = fsck.c ../bwfs/bwfs_io.c
OUT = fsck.bwfs

# stbi_write necesita -lm
all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) -lm

clean:
	rm -f $(OUT)
