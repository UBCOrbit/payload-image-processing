#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void downscale(const uint8_t *source_buf, size_t source_width, size_t source_height,
               uint8_t *dest_buf, size_t dest_width, size_t dest_height);

void downscale2(const uint8_t *source_buf, size_t source_width, size_t source_height,
               uint8_t *dest_buf, size_t dest_width, size_t dest_height);

void downscale3(const uint8_t *source_buf, size_t source_width, size_t source_height,
               uint8_t *dest_buf, size_t dest_width, size_t dest_height);

unsigned long compress(const uint8_t *buffer, size_t width, size_t height, uint8_t **out_buffer);

int compress_image(char *src_data, uint16_t in_width, uint16_t in_height, char *dst_img);