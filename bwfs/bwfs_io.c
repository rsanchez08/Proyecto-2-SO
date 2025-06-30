#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../mkfs/stb_image_write.h"
#include "bwfs.h"
#include "bwfs_io.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

int bwfs_save_image(const char *path, const void *data, size_t size) {
    
    if (!path || !data || size == 0 || size > BYTES_PER_IMAGE) {
        fprintf(stderr, "Error: Tamaño debe ser entre 1 y %d bytes\n", BYTES_PER_IMAGE);
        return -1;
    }

    size_t pixel_count = size * 8;
    size_t width = BLOCK_WIDTH;
    size_t height = BLOCK_HEIGHT;

    uint8_t *pixels = calloc(width * height, 1);
    if (!pixels) return -1;

    const uint8_t *src = (const uint8_t *)data;
    for (size_t i = 0; i < size; i++) {
        for (int b = 0; b < 8; b++) {
            size_t pixel_index = i * 8 + b;
            if (pixel_index < width * height && (src[i] & (1 << (7 - b)))) {
                pixels[pixel_index] = 255;
            }
        }
    }

    int result = stbi_write_png(path, width, height, 1, pixels, width);
    free(pixels);
    return result ? 0 : -1;
}

int bwfs_load_image(const char *path, void **data_ptr, size_t *size_ptr) {
    if (!path || !data_ptr || !size_ptr) return -1;
    
    int width, height, channels;
    uint8_t *pixels = stbi_load(path, &width, &height, &channels, 1);
    if (!pixels) return -1;

    if (width != BLOCK_WIDTH || height != BLOCK_HEIGHT) {
        fprintf(stderr, "Error: Dimensiones de imagen incorrectas (%dx%d), deben ser %dx%d\n",
                width, height, BLOCK_WIDTH, BLOCK_HEIGHT);
        stbi_image_free(pixels);
        return -1;
    }

    size_t pixel_count = width * height;
    size_t size = (pixel_count + 7) / 8;

    uint8_t *buffer = calloc(1, size);
    if (!buffer) {
        stbi_image_free(pixels);
        return -1;
    }

    for (size_t i = 0; i < pixel_count; i++) {
        if (pixels[i] > 128) {
            buffer[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    stbi_image_free(pixels);
    *data_ptr = buffer;
    *size_ptr = size;
    return 0;
}

