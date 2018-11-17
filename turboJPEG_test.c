/*
 * Build with gcc -o turboJPEG_test turboJPEG_test.c -lturbojpeg
 * 
 * This is a library test program which will compress raw image data
 * and create a JPEG image.
 */

#include <stdlib.h>
#include <stdio.h>
#include <turbojpeg.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argv[1] == NULL)
    {
        printf("Usage: ./path/to/program ./path/to/source/data ./path/to/output/jpeg\n");
        return 0;
    }

    int err;

    // Image parameters
    int JPEG_QUALITY = 25;
    const int COLOR_COMPONENTS = 3;
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
    for (int i = 0; i < sz; i++)
    {
        buffer[i] = (char) fgetc(in_file);
    }

    // Don't need source data anymore.
    fclose(in_file);

    printf("Read in file of size: %d kb.\n", (int) sz / 1000);

    tjhandle jpeg_compressor = tjInitCompress();

    printf("Compressing...\nSize    Pass\n");
    do
    {
        tjCompress2(jpeg_compressor, buffer, width, 0, height, TJPF_RGB,
                    &out_buffer, &jpeg_size, TJSAMP_444, JPEG_QUALITY,
                    TJFLAG_FASTDCT);
        
        printf("%4lukb   %4d\n", jpeg_size/1000, pass);
        pass++;
        JPEG_QUALITY -= 1;
    }
    while (JPEG_QUALITY > 0 && jpeg_size > 100000);

    printf("Final image size: %d kb.\n", (int) jpeg_size / 1000);

    tjDestroy(jpeg_compressor);

    // Open or create the output jpeg for writing.
    FILE *out_file = fopen(argv[2], "w+");

    // Write the compressed image data to the output file.
    for (int i = 0; i < jpeg_size; i++)
    {

        fputc((int) out_buffer[i], out_file);
    }

    // Done writing to file.
    fclose(out_file);

    //to free the memory allocated by TurboJPEG (either by tjAlloc(),
    //or by the Compress/Decompress) after you are done working on it:
    tjFree(out_buffer);

    free(buffer);

    return 0;
}

