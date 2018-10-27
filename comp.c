/*
 * This program is a command line wrapper which will compress the 
 * jpeg image at a certain path to a specified amount.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>

// static int exec_prog(const char **argv);

int main(int argc, char const *argv[])
{
    char *imagePath = "highres-1280-960-test.jpg "; // The path to the image to compress.
    char *options = "-S 100 "; // Choose the desired size. Units in kb.
    char cmdString[100]; // The string to execute.

    strcat(cmdString, "jpegoptim ");
    strcat(cmdString, imagePath);
    strcat(cmdString, options);

    // Execute the command.
    int status = system(cmdString);

    return 0;
}
