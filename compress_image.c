/*
 * Loads raw pixel data, converts it to a JPEG image, and compresses it to
 * under a specified limit.
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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

    double sq_factor = width_factor * height_factor;

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

            dest[y][x][0] = avg_r / sq_factor;
            dest[y][x][1] = avg_g / sq_factor;
            dest[y][x][2] = avg_b / sq_factor;
        }
    }
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
