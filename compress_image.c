/*
 * Loads raw pixel data, converts it to a JPEG image, and compresses it to
 * under a specified limit.
 */

#include <stdlib.h>
#include <stdio.h>
#include <turbojpeg.h>
#include <string.h>

#define MAX_IMG_SIZE 100*1024
#define COLOR_COMPONENTS 3

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: %s ./path/to/source/data ./path/to/output/jpeg\n",
               argv[0]);
        return 0;
    }

    size_t err;

    // Image parameters
    int jpeg_quality = 25;
    int width = 1280;
    int height = 960;
    long unsigned int jpeg_size = 0;
    int pass = 1;

    // File buffers
    unsigned char* buffer;
    unsigned char* out_buffer = NULL;

    // Open the raw data file for reading.
    FILE *in_file = fopen(argv[1], "r");

    // Determine the size of the file.
    fseek(in_file, 0L, SEEK_END);
    size_t sz = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);

    buffer = malloc(sz);

    // Read in the raw data into the buffer.
    err = fread(buffer, 1, sz, in_file);
    if (err != sz) {
        printf("Error reading raw data.\n");
        return -1;
    }

    // Don't need source data anymore.
    fclose(in_file);

    printf("Read in file of size: %d kb.\n", (int) sz / 1024);

    tjhandle jpeg_compressor = tjInitCompress();

    printf("Compressing...\nSize    Pass\n");
    do
    {
        tjCompress2(jpeg_compressor, buffer, width, 0, height, TJPF_RGB,
                    &out_buffer, &jpeg_size, TJSAMP_444, jpeg_quality,
                    TJFLAG_FASTDCT);

        printf("%4lukb   %4d\n", jpeg_size/1024, pass);
        pass++;
        jpeg_quality -= 1;
    }
    while (jpeg_quality > 0 && jpeg_size > MAX_IMG_SIZE);

    printf("Final image size: %d kb.\n", (int) jpeg_size / 1024);

    tjDestroy(jpeg_compressor);

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

    return 0;
}
