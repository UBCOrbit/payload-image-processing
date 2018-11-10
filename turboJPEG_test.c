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
        printf("Usage: [program name] [source image] [output image]\n");
        return 0;
    }
    int err;
    const int JPEG_QUALITY = 75;
    const int COLOR_COMPONENTS = 3;
    int _width = 1280;
    int _height = 960;
    long unsigned int _jpegSize = 0;
    
    unsigned char* buffer;
    unsigned char* out_buffer = NULL;

    FILE *in_file = fopen(argv[1], "r"); // Uncompressed image data
    FILE *out_file = fopen(argv[2], "w+");

    fseek(in_file, 0L, SEEK_END);
    size_t sz = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);
    
    buffer = malloc(sz);

    for (int i = 0; i < sz; i++)
    {
        buffer[i] = (char) fgetc(in_file);
    }

    // Don't need source data anymore.
    fclose(in_file);

    printf("Read in file of size: %d kb.\n", (int) sz / 1000);

    tjhandle _jpegCompressor = tjInitCompress();

    tjCompress2(_jpegCompressor, buffer, _width, 0, _height, TJPF_RGB,
                &out_buffer, &_jpegSize, TJSAMP_444, JPEG_QUALITY,
                TJFLAG_FASTDCT);

    printf("Compressed image to size: %d kb.\n", (int) _jpegSize / 1000);

    tjDestroy(_jpegCompressor);

    // Write the compressed image data to the output file.
    for (int i = 0; i < _jpegSize; i++)
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

