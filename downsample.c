
/*
 * This function will use the passed pixel buffer and downsample it
 * into the downsample buffer.
 * 
 * Algorithm:
 * - Calculate how much to crop off each row and how many rows to remove
 * - - This will be based on the ratio of the input image
 */

#define IN_WIDTH 5664
#define IN_HEIGHT 4248

#define OUT_WIDTH 1280 
#define OUT_HEIGHT 960

int downsample(unsigned char *pixel_buffer, unsigned char *ds_buffer)
{
    int ds_factor = IN_WIDTH / OUT_WIDTH;

    if (ds_factor % 2 != 0)
    {
        ds_factor++; // Factor it more instead of less to ensure the size requirement is met.
    }

    const int cropped_width = OUT_WIDTH * ds_factor;
    const int cropped_height = OUT_HEIGHT * ds_factor;

    printf("Cropping input image to %dx%d pixels.\n", cropped_width, cropped_height);

    // Allocate space for the first column of pixels.
    unsigned char** cropped_image = malloc(cropped_height * sizeof(char*));

    // Initialize the 2D array
    for (int i = 0; i < cropped_height; i++)
    {
        // Allocate space for the row of RGB data.
        cropped_image[i] = malloc(cropped_width * sizeof(unsigned char) * 3);
        
        // Insert the RGB data from the pixel_buffer to the new 2D array.
        for (int j = 0; j < cropped_width * 3; j++)
        {
            cropped_image[i][j] = pixel_buffer[j + (i * IN_WIDTH * 3)];
        }
    }

    printf("Allocated space for cropped image.\n");

    // Free the 2D array
    for (int i = 0; i < cropped_height; i++)
    {
        free(cropped_image[i]);
    }

    free(cropped_image);

    return 0;
}