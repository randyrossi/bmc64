#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>

void main(int argc, char **argv)
{
    FILE *output;

    if (argc < 2) {
        printf("Usage: genmsvcver output-filename\n");
        exit(0);
    }
    output = fopen(argv[1], "wt");
    if (output) {
        fprintf(output, "#define MSVC_VER %d\n", _MSC_VER);
        fclose(output);
    }
}
