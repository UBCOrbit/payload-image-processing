/*
 * Loads raw pixel data, converts it to a JPEG image, and compresses it to
 * under a specified limit.
 */

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <turbojpeg.h>

#define MAX_IMG_SIZE 100000

// #define IN_WIDTH 5664
// #define IN_HEIGHT 4248
#define IN_WIDTH 5505
#define IN_HEIGHT 3670

#define OUT_WIDTH 1280
#define OUT_HEIGHT 853

void downscale(const uint8_t *source_buf, size_t source_width, size_t source_height,
               uint8_t *dest_buf, size_t dest_width, size_t dest_height)
{
    const uint8_t (*source)[source_width][3] = (void*) source_buf;
    uint8_t (*dest)[dest_width][3] = (void*) dest_buf;

    /*
     * amount of pixels in source that a pixel in dest covers, should always be
     * greater than 1. Might break horribly if it isn't.
     */
    double width_factor = ((double) source_width) / dest_width;
    double height_factor = ((double) source_height) / dest_height;

    double sq_factor = 1.0 / (width_factor * height_factor);

    for (uint32_t y = 0; y < dest_height; ++y) {
        double lower_y = height_factor * y;
        double upper_y = lower_y + height_factor;
        uint32_t start_y = floor(lower_y);
        uint32_t end_y = ceil(upper_y);

        for (uint32_t x = 0; x < dest_width; ++x) {
            double lower_x = width_factor * x;
            double upper_x = lower_x + width_factor;
            uint32_t start_x = floor(lower_x);
            uint32_t end_x = ceil(upper_x);

            double avg_r = 0;
            double avg_g = 0;
            double avg_b = 0;

            for (uint32_t sy = start_y; sy < end_y; ++sy) {
                double len_y = fmin(sy + 1, upper_y) - fmax(sy, lower_y);

                for (uint32_t sx = start_x; sx < end_x; ++sx) {
                    double len_x = fmin(sx + 1, upper_x) - fmax(sx, lower_x);
                    double area = len_x * len_y;
                    // average plus equals the color times pixel area
                    avg_r += area * source[sy][sx][0];
                    avg_g += area * source[sy][sx][1];
                    avg_b += area * source[sy][sx][2];
                }
            }

            dest[y][x][0] = avg_r * sq_factor + 0.5;
            dest[y][x][1] = avg_g * sq_factor + 0.5;
            dest[y][x][2] = avg_b * sq_factor + 0.5;
        }
    }
}

// Testing new algorithm that might be faster at the cost of more memory
void downscale2(const uint8_t *source_buf, size_t source_width, size_t source_height,
               uint8_t *dest_buf, size_t dest_width, size_t dest_height)
{
    const uint8_t (*source)[source_width][3] = (void*) source_buf;
    double (*dest)[dest_width][3] = calloc(dest_width * dest_height * 3, sizeof(double));

    // Note, these are the inverse of the previous algorithm, should always be < 1
    double width_factor = ((double) dest_width) / source_width;
    double height_factor = ((double) dest_height) / source_height;

    double sq_factor = width_factor * height_factor;

    for (uint32_t sy = 0; sy < source_height; ++sy) {
        uint32_t y_min = sy * height_factor; // floor by default
        uint32_t y_max = (sy + 1) * height_factor;
        if (y_min == y_max || y_max == dest_height) {
            // Pixel y lies entirely within one pixel of output
            for (uint32_t sx = 0; sx < source_width; ++sx) {
                uint32_t x_min = sx * width_factor; // floor by default
                uint32_t x_max = (sx + 1) * width_factor;
                if (x_min == x_max || x_max == dest_width) {
                    // Pixel x lies entirely within one pixel of output
                    dest[y_min][x_min][0] += source[sy][sx][0];
                    dest[y_min][x_min][1] += source[sy][sx][1];
                    dest[y_min][x_min][2] += source[sy][sx][2];
                } else {
                    // Pixel x straddles two pixels
                    double min_x_area = x_max / width_factor - sx;
                    double max_x_area = 1 - min_x_area;

                    dest[y_min][x_min][0] += source[sy][sx][0] * min_x_area;
                    dest[y_min][x_min][1] += source[sy][sx][1] * min_x_area;
                    dest[y_min][x_min][2] += source[sy][sx][2] * min_x_area;

                    dest[y_min][x_max][0] += source[sy][sx][0] * max_x_area;
                    dest[y_min][x_max][1] += source[sy][sx][1] * max_x_area;
                    dest[y_min][x_max][2] += source[sy][sx][2] * max_x_area;
                }
            }
        } else {
            // Pixel y straddles two pixels
            double min_y_area = y_max / height_factor - sy;
            double max_y_area = 1 - min_y_area;

            for (uint32_t sx = 0; sx < source_width; ++sx) {
                uint32_t x_min = sx * width_factor; // floor by default
                uint32_t x_max = (sx + 1) * width_factor;
                if (x_min == x_max || x_max == dest_width) {
                    // Pixel x lies entirely within one pixel of output
                    dest[y_min][x_min][0] += source[sy][sx][0] * min_y_area;
                    dest[y_min][x_min][1] += source[sy][sx][1] * min_y_area;
                    dest[y_min][x_min][2] += source[sy][sx][2] * min_y_area;

                    dest[y_max][x_min][0] += source[sy][sx][0] * max_y_area;
                    dest[y_max][x_min][1] += source[sy][sx][1] * max_y_area;
                    dest[y_max][x_min][2] += source[sy][sx][2] * max_y_area;
                } else {
                    // Pixel x straddles two pixels
                    double min_x_area = x_max / width_factor - sx;
                    double max_x_area = 1 - min_x_area;

                    dest[y_min][x_min][0] += source[sy][sx][0] * min_y_area * min_x_area;
                    dest[y_min][x_min][1] += source[sy][sx][1] * min_y_area * min_x_area;
                    dest[y_min][x_min][2] += source[sy][sx][2] * min_y_area * min_x_area;

                    dest[y_max][x_min][0] += source[sy][sx][0] * max_y_area * min_x_area;
                    dest[y_max][x_min][1] += source[sy][sx][1] * max_y_area * min_x_area;
                    dest[y_max][x_min][2] += source[sy][sx][2] * max_y_area * min_x_area;

                    dest[y_min][x_max][0] += source[sy][sx][0] * min_y_area * max_x_area;
                    dest[y_min][x_max][1] += source[sy][sx][1] * min_y_area * max_x_area;
                    dest[y_min][x_max][2] += source[sy][sx][2] * min_y_area * max_x_area;

                    dest[y_max][x_max][0] += source[sy][sx][0] * max_y_area * max_x_area;
                    dest[y_max][x_max][1] += source[sy][sx][1] * max_y_area * max_x_area;
                    dest[y_max][x_max][2] += source[sy][sx][2] * max_y_area * max_x_area;
                }
            }
        }
    }

    double *dest_arr = (void*) dest;
    for (size_t i = 0; i < 3 * dest_width * dest_height; ++i) {
        dest_buf[i] = dest_arr[i] * sq_factor + 0.5;
    }

    free(dest);
}

void downscale3(const uint8_t *source_buf, size_t source_width, size_t source_height,
               uint8_t *dest_buf, size_t dest_width, size_t dest_height)
{
    const uint8_t (*source)[source_width][3] = (void*) source_buf;
    double (*dest)[dest_width][3] = calloc(dest_width * dest_height * 3, sizeof(double));

    // Note, these are the inverse of the previous algorithm, should always be < 1
    double width_factor = ((double) dest_width) / source_width;
    double height_factor = ((double) dest_height) / source_height;

    double sq_factor = width_factor * height_factor;

    double *x_widths_min = malloc(source_width * sizeof(double));
    double *x_widths_max = malloc(source_width * sizeof(double));

    for (size_t i = 0; i < source_width; ++i) {
        uint32_t x_min = i * width_factor; // floor by default
        uint32_t x_max = (i + 1) * width_factor;
        if (x_min == x_max || x_max == dest_width) {
            x_widths_min[i] = 1.0;
        } else {
            x_widths_min[i] = x_max / width_factor - i;
        }
        x_widths_max[i] = 1 - x_widths_min[i];
    }

    for (uint32_t sy = 0; sy < source_height; ++sy) {
        uint32_t y_min = sy * height_factor; // floor by default
        uint32_t y_max = (sy + 1) * height_factor;
        if (y_min == y_max || y_max == dest_height) {
            // Pixel y lies entirely within one pixel of output
            for (uint32_t sx = 0; sx < source_width; ++sx) {
                uint32_t x_min = sx * width_factor; // floor by default
                if (x_widths_min[sx] == 1.0) {
                    // Pixel x lies entirely within one pixel of output
                    dest[y_min][x_min][0] += source[sy][sx][0];
                    dest[y_min][x_min][1] += source[sy][sx][1];
                    dest[y_min][x_min][2] += source[sy][sx][2];
                } else {
                    dest[y_min][x_min][0] += source[sy][sx][0] * x_widths_min[sx];
                    dest[y_min][x_min][1] += source[sy][sx][1] * x_widths_min[sx];
                    dest[y_min][x_min][2] += source[sy][sx][2] * x_widths_min[sx];

                    uint32_t x_max = (sx + 1) * width_factor;
                    dest[y_min][x_max][0] += source[sy][sx][0] * x_widths_max[sx];
                    dest[y_min][x_max][1] += source[sy][sx][1] * x_widths_max[sx];
                    dest[y_min][x_max][2] += source[sy][sx][2] * x_widths_max[sx];
                }
            }
        } else {
            // Pixel y straddles two pixels
            double min_y_area = y_max / height_factor - sy;
            double max_y_area = 1 - min_y_area;

            for (uint32_t sx = 0; sx < source_width; ++sx) {
                uint32_t x_min = sx * width_factor; // floor by default
                if (x_widths_min[sx] == 1.0) {
                    // Pixel x lies entirely within one pixel of output
                    dest[y_min][x_min][0] += source[sy][sx][0] * min_y_area;
                    dest[y_min][x_min][1] += source[sy][sx][1] * min_y_area;
                    dest[y_min][x_min][2] += source[sy][sx][2] * min_y_area;

                    dest[y_max][x_min][0] += source[sy][sx][0] * max_y_area;
                    dest[y_max][x_min][1] += source[sy][sx][1] * max_y_area;
                    dest[y_max][x_min][2] += source[sy][sx][2] * max_y_area;
                } else {
                    // Pixel x straddles two pixels
                    dest[y_min][x_min][0] += source[sy][sx][0] * min_y_area * x_widths_min[sx];
                    dest[y_min][x_min][1] += source[sy][sx][1] * min_y_area * x_widths_min[sx];
                    dest[y_min][x_min][2] += source[sy][sx][2] * min_y_area * x_widths_min[sx];

                    dest[y_max][x_min][0] += source[sy][sx][0] * max_y_area * x_widths_min[sx];
                    dest[y_max][x_min][1] += source[sy][sx][1] * max_y_area * x_widths_min[sx];
                    dest[y_max][x_min][2] += source[sy][sx][2] * max_y_area * x_widths_min[sx];

                    uint32_t x_max = (sx + 1) * width_factor;
                    dest[y_min][x_max][0] += source[sy][sx][0] * min_y_area * x_widths_max[sx];
                    dest[y_min][x_max][1] += source[sy][sx][1] * min_y_area * x_widths_max[sx];
                    dest[y_min][x_max][2] += source[sy][sx][2] * min_y_area * x_widths_max[sx];

                    dest[y_max][x_max][0] += source[sy][sx][0] * max_y_area * x_widths_max[sx];
                    dest[y_max][x_max][1] += source[sy][sx][1] * max_y_area * x_widths_max[sx];
                    dest[y_max][x_max][2] += source[sy][sx][2] * max_y_area * x_widths_max[sx];
                }
            }
        }
    }

    double *dest_arr = (void*) dest;
    for (size_t i = 0; i < 3 * dest_width * dest_height; ++i) {
        dest_buf[i] = dest_arr[i] * sq_factor + 0.5;
    }

    free(dest);
    free(x_widths_max);
    free(x_widths_min);
}

size_t compress(const uint8_t *buffer, size_t width, size_t height, uint8_t **out_buffer)
{
    // Image parameters
    int jpeg_quality = 25;
    size_t jpeg_size = 0;
    int pass = 1;

    tjhandle jpeg_compressor = tjInitCompress();

    printf("Compressing...\nSize    Pass\n");

    do {
        tjCompress2(jpeg_compressor, buffer, width, 0, height, TJPF_RGB,
                    out_buffer, &jpeg_size, TJSAMP_420, jpeg_quality,
                    TJFLAG_FASTDCT);

        printf("%4lukb   %4d\n", jpeg_size / 1024, pass);
        pass++;
        jpeg_quality--;
    } while (jpeg_quality > 0 && jpeg_size > MAX_IMG_SIZE);

    printf("Final image size: %d kb.\n", (int) jpeg_size / 1024);

    tjDestroy(jpeg_compressor);

    return jpeg_size;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: %s ./path/to/source/data ./path/to/output/jpeg\n",
               argv[0]);
        return 0;
    }

    size_t err;

    // File buffers
    uint8_t *buffer;
    uint8_t *ds_buffer;
    uint8_t *out_buffer = NULL;

    // Open the raw data file for reading.
    FILE *in_file = fopen(argv[1], "r");

    // Determine the size of the file.
    // Later this will be a constant size for our images
    fseek(in_file, 0L, SEEK_END);
    size_t sz = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);

    if (sz != IN_WIDTH * IN_HEIGHT * 3) {
        printf("Invalid file size.\n");
        return -1;
    }

    buffer = malloc(sz);

    // Read in the raw data into the buffer.
    err = fread(buffer, 1, sz, in_file);
    if (err != sz) {
        printf("Error reading raw data.\n");
        return -1;
    }

    printf("Read in file of size: %d kb.\n", (int) sz / 1024);

    // Don't need source data anymore.
    fclose(in_file);

    ds_buffer = malloc(OUT_WIDTH * OUT_HEIGHT * 3);

    /*
    uint8_t *ds2_buffer = malloc(OUT_WIDTH * OUT_HEIGHT * 3);

    downscale2(buffer, IN_WIDTH, IN_HEIGHT, ds_buffer, OUT_WIDTH, OUT_HEIGHT);
    downscale3(buffer, IN_WIDTH, IN_HEIGHT, ds2_buffer, OUT_WIDTH, OUT_HEIGHT);

    for (size_t i = 0; i < OUT_WIDTH * OUT_HEIGHT * 3; ++i) {
        if (ds_buffer[i] != ds2_buffer[i]) {
            printf("differing pixel at: %lu, (%lu, %lu, %lu), %d, %d\n",
                   i, i / (OUT_WIDTH * 3), (i % OUT_WIDTH) / 3, i % 3,
                   ds_buffer[i], ds2_buffer[i]);
        }
    }
    free(ds2_buffer);
    */

    /*
    struct timespec t1, t2;
    double start_time, stop_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < 100; ++i)
        downscale(buffer, IN_WIDTH, IN_HEIGHT, ds_buffer, OUT_WIDTH, OUT_HEIGHT);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);

    start_time = t1.tv_sec + t1.tv_nsec / 1000000000.0;
    stop_time = t2.tv_sec + t2.tv_nsec / 1000000000.0;
    printf("Downscale: %f\n", stop_time - start_time);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < 100; ++i)
        downscale2(buffer, IN_WIDTH, IN_HEIGHT, ds_buffer, OUT_WIDTH, OUT_HEIGHT);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);

    start_time = t1.tv_sec + t1.tv_nsec / 1000000000.0;
    stop_time = t2.tv_sec + t2.tv_nsec / 1000000000.0;
    printf("Downscale2: %f\n", stop_time - start_time);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < 100; ++i)
        downscale3(buffer, IN_WIDTH, IN_HEIGHT, ds_buffer, OUT_WIDTH, OUT_HEIGHT);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);

    start_time = t1.tv_sec + t1.tv_nsec / 1000000000.0;
    stop_time = t2.tv_sec + t2.tv_nsec / 1000000000.0;
    printf("Downscale3: %f\n", stop_time - start_time);
    */

    downscale(buffer, IN_WIDTH, IN_HEIGHT, ds_buffer, OUT_WIDTH, OUT_HEIGHT);

    size_t jpeg_size = compress(ds_buffer, OUT_WIDTH, OUT_HEIGHT, &out_buffer);

    // Open or create the output jpeg for writing.
    FILE *out_file = fopen(argv[2], "w");

    // Write the compressed image data to the output file.
    err = fwrite(out_buffer, 1, jpeg_size, out_file);
    if (err != jpeg_size) {
        printf("Error writing compressed image.\n");
        return -1;
    }

    // Done writing to file.
    fclose(out_file);

    //to free the memory allocated by TurboJPEG (either by tjAlloc(),
    //or by the Compress/Decompress) after you are done working on it:
    tjFree(out_buffer);

    free(buffer);
    free(ds_buffer);

    return 0;
}
