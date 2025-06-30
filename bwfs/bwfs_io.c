// Define para incluir la implementación de escritura de imágenes PNG
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../mkfs/stb_image_write.h"

// Define para incluir la implementación de lectura de imágenes PNG
#define STB_IMAGE_IMPLEMENTATION
#include "../mkfs/stb_image.h"

#include "bwfs.h"
#include "bwfs_io.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/*
 * Función: bwfs_save_image
 * -----------------------------------
 * Guarda un bloque de datos binarios como una imagen PNG en blanco y negro.
 *
 * path: Ruta donde se guardará la imagen.
 * data: Puntero al buffer de datos (en bytes).
 * size: Tamaño en bytes del buffer.
 *
 * Retorna 0 si se guarda correctamente, -1 en caso de error.
 */
int bwfs_save_image(const char *path, const void *data, size_t size) {
    // Validación de parámetros
    if (!path || !data || size == 0 || size > BYTES_PER_IMAGE) {
        fprintf(stderr, "Error: Tamaño debe ser entre 1 y %d bytes\n", BYTES_PER_IMAGE);
        return -1;
    }

    // Cantidad total de píxeles que se representarán (8 bits por byte)
    size_t pixel_count = size * 8;
    size_t width = BLOCK_WIDTH;
    size_t height = BLOCK_HEIGHT;

    // Se inicializa un arreglo de píxeles en 0 (negro)
    uint8_t *pixels = calloc(width * height, 1);
    if (!pixels) return -1;

    // Convertir los datos en pixeles blanco/negro (1 bit por pixel)
    const uint8_t *src = (const uint8_t *)data;
    for (size_t i = 0; i < size; i++) {
        for (int b = 0; b < 8; b++) {
            size_t pixel_index = i * 8 + b;
            if (pixel_index < width * height && (src[i] & (1 << (7 - b)))) {
                pixels[pixel_index] = 255; // Blanco
            }
        }
    }

    // Guardar la imagen PNG en escala de grises
    int result = stbi_write_png(path, width, height, 1, pixels, width);

    // Liberar memoria
    free(pixels);

    // Retornar resultado (0 éxito, -1 error)
    return result ? 0 : -1;
}

/*
 * Función: bwfs_load_image
 * -----------------------------------
 * Carga una imagen PNG y la interpreta como datos binarios (1 bit por píxel).
 *
 * path: Ruta del archivo de imagen.
 * data_ptr: Puntero donde se almacenará el buffer de salida.
 * size_ptr: Puntero donde se almacenará el tamaño del buffer resultante.
 *
 * Retorna 0 si se carga correctamente, -1 en caso de error.
 */
int bwfs_load_image(const char *path, void **data_ptr, size_t *size_ptr) {
    // Validar parámetros de entrada
    if (!path || !data_ptr || !size_ptr) return -1;

    int width, height, channels;

    // Cargar imagen
    uint8_t *pixels = stbi_load(path, &width, &height, &channels, 1);
    if (!pixels) return -1;

    // Verificar dimensiones correctas
    if (width != BLOCK_WIDTH || height != BLOCK_HEIGHT) {
        fprintf(stderr, "Error: Dimensiones de imagen incorrectas (%dx%d), deben ser %dx%d\n",
                width, height, BLOCK_WIDTH, BLOCK_HEIGHT);
        stbi_image_free(pixels);
        return -1;
    }

    // Calcular el tamaño del buffer (1 bit por píxel → 8 píxeles por byte)
    size_t pixel_count = width * height;
    size_t size = (pixel_count + 7) / 8;

    // Asignar memoria para almacenar los datos resultantes
    uint8_t *buffer = calloc(1, size);
    if (!buffer) {
        stbi_image_free(pixels);
        return -1;
    }

    // Convertir píxeles en bits (pixeles > 128 se consideran blancos)
    for (size_t i = 0; i < pixel_count; i++) {
        if (pixels[i] > 128) {
            buffer[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    // Liberar memoria de la imagen cargada
    stbi_image_free(pixels);

    // Asignar punteros de salida
    *data_ptr = buffer;
    *size_ptr = size;

    return 0;
}
