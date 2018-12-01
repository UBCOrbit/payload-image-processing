#include <stdio.h>
#include <stdlib.h>
#include "./compress_image.h"

void main()
{
    printf("Running test suite...\n");
    // system("./compress_image ./Images/Satellite/mcmurray-5505-3670-original.data 5505 3670 ./out.jpg");
    // system("./compress_image ./Images/Marina/marina-5664-4248-original.data 5664 4248 ./out.jpg");
 
    compress_image("./Images/Satellite/mcmurray-5505-3670-original.data", 5505, 3670, "./out.jpg");
} 