/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen :  */

/** \file   bin2c.c
 * \brief   bin2c.c - binary to c array converter.
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

/* Usage:
 *
 *       argv[1]  argv[2]   argv[3]
 *
 * bin2c <infile> <outfile> <array-decl>
 *
 * Please note: after opening both infile and outfile, no additional error
 * checks are done when reading or writing data. So please don't use this code
 * in medical equipment or other critical systems :)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/** \brief  Number of bytes to dump per C source line
 *
 * This value sets the number of bytes to dump per C source file line. Used
 * when the array is a single dimension, if the array 
 */
#define BYTES_PER_LINE  8


/*
 * forward declaration of static functions
 */
static void     usage(void);
static size_t   parse_dimension(char *dim, char **endptr);
static int      parse_array_decl(const char *decl, size_t *d1, size_t *d2);
static void     write_array_start(const char *decl);
static void     write_array_end(void);
static void     write_array_data(void);
static void     write_array_data_1d(void);
static void     write_array_data_2d(void);

/** \brief  File handle for input file
 *
 * The input file is a binary blob
 *
 * */
static FILE *infile = NULL;

/** \brief  File handle for output file
 *
 * The output file is a C source file
 */
static FILE *outfile = NULL;

/** \brief  Array name to use for the binary blob dump
 *
 * This is just the name of the array, no dimensions are present
 */
static char *arrayname = NULL;

/** \brief  Size of the first array dimension
 *
 * Optional, if 0 the array declaration will be [], otherwise [dim1]
 */
static size_t dim1 = 0;


/** \brief  Size of the second array dimension
 *
 * Optional, if 0 the array declaration will be [dim1][], otherwise [dim1][dim2]
 */
static size_t dim2 = 0;


/** \brief  Total size of array
 *
 * Used for checking the number of bytes read against the dimensions of the
 * array
 */
static size_t total = 0;


/** \brief  Try to parse a fixed array dimension decl
 *
 *
 * The \a dim argument is supposed to point to the char after the opening
 * bracket ('[') of a dimension declaration.
 *
 * \param[in]   dim     array dimension declaration
 * \param[out]  endptr  pointer to first non-consumed char
 *
 * \return  array size, or 0 when either '[]' or '[SOME_CONST]'
 */
static size_t parse_dimension(char *dim, char **endptr)
{
    long result;

    if (*dim == ']') {
        /* variable length */
        *endptr = dim;
        return 0;
    }

    result = strtol(dim, endptr, 0);
    if (**endptr == ']') {
        /* OK */
        return (size_t)result;
    }

    /* probably a #define inside [] */
    *endptr = strchr(dim, ']');
    return 0;
}




/** \brief  Parse the array declaration for fixed dimensions
 *
 * \param[in]   decl    array declaration
 * \param[out]  d1      first dimension
 * \param[out]  d2      second dimension
 *
 * \return 0 on success, -1 on failure
 */
static int parse_array_decl(const char *decl, size_t *d1, size_t *d2)
{
    char *end;
    char *s;

    s = strchr(decl, '[');
    if (s == NULL) {
        fprintf(stderr, "bin2c: failed to parse array declaration\n");
        return -1;
    }

    s++;
    /* try to parse the first dimension */
    *d1 = parse_dimension(s, &end);
    if (*end != ']') {
        return -1;
    }
    s = end + 1;
    if (*s == '[') {
        /* second dimension given, parse */
        s++;
        *d2 = parse_dimension(s, &end);
        if (*end != ']') {
            return -1;
        }
    } else {
        *d2 = 0;
    }
    return 0;
}



/** \brief  Display usage/help message
 */
static void usage(void)
{
    printf("Usage: bin2c <infile> <outfile> <array-decl>\n\n");
    printf("Where <infile> is the binary file to convert to C, <outfile> is the\n");
    printf("C source file to generate and <array-decl> is the array declaration to\n");
    printf("use in the C source file.\n\n");
}



/** \brief  Write array declaration start to C file
 *
 * \param[in]   decl    array declaration, without the '= {'
 */
static void write_array_start(const char *decl)
{
    fprintf(outfile, "%s = {\n", decl);
}


/** \brief  Write array declaration end to C file
 */
static void write_array_end(void)
{
    fprintf(outfile, "\n};\n");
}


/** \brief  Write a one-dimensional array of the data
 */
static void write_array_data_1d(void)
{
    int col;
    int start  = 1;

    col = 0;
    fprintf(outfile, "    ");
    while (1) {
        int b = fgetc(infile);
        if (feof(infile)) {
            return;
        }
        total++;

        if (col == BYTES_PER_LINE) {
            fprintf(outfile, ",\n    ");
            col = 0;
        } else {
            if (start) {
                start = 0;
            } else {
                fprintf(outfile, ", ");
            }
        }

        fprintf(outfile, "0x%02x", b);
        col++;
    }
}


/** \brief  Write a two-dimensional array of the data
 */
static void write_array_data_2d(void)
{
    int col;
    int start  = 1;

    col = 0;
    fprintf(outfile, "    { ");
    while (1) {
        int b = fgetc(infile);
        if (feof(infile)) {
            fprintf(outfile, " }");
            return;
        }
        total++;

        if (col == (int)dim2) {
            fprintf(outfile, " },\n    { ");
            col = 0;
        } else {
            if (start) {
                start = 0;
            } else {
                fprintf(outfile, ", ");
            }
        }

        fprintf(outfile, "0x%02x", b);
        col++;
    }
}


/** \brief  Write input data as C array to outfile
 */
static void write_array_data(void)
{
    if (dim2 == 0) {
        write_array_data_1d();
    } else {
        write_array_data_2d();
    }
}


/* ------------------- entry point, command line 'parser' -------------------*/

/** \brief  Driver
 *
 * \param[in]   argc    argument count
 * \param[in]   argv    argument vector
 *
 * \return  `EXIT_SUCCESS` or `EXIT_FAILURE`
 */
int main(int argc, char **argv)
{
    if (argc < 4 || (argc >=2 && strcmp(argv[1], "--help") == 0)) {
        usage();
        return EXIT_SUCCESS;
    }

    /* try to open both files */
    infile = fopen(argv[1], "rb");
    if (infile == NULL) {
        fprintf(stderr, "bin2c: failed to open '%s' for reading: %d: %s\n",
                argv[1], errno, strerror(errno));
        return EXIT_FAILURE;
    }
    outfile = fopen(argv[2], "wb");
    if (outfile == NULL) {
        fprintf(stderr, "bin2c: failed to open '%s' for writing: %d: %s\n",
                argv[2], errno, strerror(errno));
        fclose(infile);
        return EXIT_FAILURE;
    }

    /* get array name */
    arrayname = argv[3];

    if (parse_array_decl(argv[3], &dim1, &dim2) < 0) {
        fprintf(stderr, "bin2c: failed to parse array declation '%s'\n",
                argv[3]);
        fclose(infile);
        fclose(outfile);
        return EXIT_FAILURE;
    }
#if 0
    printf("dim1 = %lu\n", (unsigned long)dim1);
    printf("dim2 = %lu\n", (unsigned long)dim2);
#endif
    write_array_start(argv[3]);
    write_array_data();
    write_array_end();

#if 0
    printf("Done, read a total of %lu bytes\n", total);
#endif
    /* check bytes read against array dimension */
    if (dim1 > 0 && dim2 > 0) {
        if (dim1 * dim2 != total) {
            fprintf(stderr,
                    "bin2c: WARNING: number of bytes read (%lu) does not match"
                    " array dimensions (%lu*%lu == %lu)\n",
                    (unsigned long)total,
                    (unsigned long)dim1,
                    (unsigned long)dim2,
                    (unsigned long)(dim1 * dim2));
        }
    } else if (dim1 > 0 && dim2 == 0) {
        if (dim1 != total) {
            fprintf(stderr,
                    "bin2c: WARNING: number of bytes read (%lu) does not match"
                    " array dimension (%lu)\n",
                    (unsigned long)total,
                    (unsigned long)dim1);
        }
    }

    /* clean up */
    fclose(infile);
    fclose(outfile);
    return EXIT_SUCCESS;
}

