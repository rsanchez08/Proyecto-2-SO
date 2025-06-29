CC = gcc
CFLAGS = -Wall -Wextra -I../bwfs -I../mkfs -g -std=c11
LDLIBS = -lm

SRC = mkfs.c ../bwfs/bwfs_io.c

all: mkfs.bwfs

mkfs.bwfs: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDLIBS)

clean:
	rm -f mkfs.bwfs

.PHONY: all clean