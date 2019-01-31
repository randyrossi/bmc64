/*
 * dat2h.c - resid dat file to header conversion program.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static FILE *infile = NULL;
static FILE *outfile = NULL;
static unsigned char buffer[4096];
static int counter = 0;
static char *inoutput_path = NULL;

typedef struct names_s {
    char *inname;
    char *outname;
} names_t;

static names_t names[] = {
    { "wave6581__ST.dat", "wave6581__ST.h" },
    { "wave8580__ST.dat", "wave8580__ST.h" },
    { "wave6581_P_T.dat", "wave6581_P_T.h" },
    { "wave8580_P_T.dat", "wave8580_P_T.h" },
    { "wave6581_PS_.dat", "wave6581_PS_.h" },
    { "wave8580_PS_.dat", "wave8580_PS_.h" },
    { "wave6581_PST.dat", "wave6581_PST.h" },
    { "wave8580_PST.dat", "wave8580_PST.h" },
    { NULL, NULL }
};

static void write_header(void)
{
    fprintf(outfile, "//  ---------------------------------------------------------------------------\n");
    fprintf(outfile, "//  This file is part of reSID, a MOS6581 SID emulator engine.\n");
    fprintf(outfile, "//  Copyright (C) 2010  Dag Lem <resid@nimrod.no>\n");
    fprintf(outfile, "//\n");
    fprintf(outfile, "//  This program is free software; you can redistribute it and/or modify\n");
    fprintf(outfile, "//  it under the terms of the GNU General Public License as published by\n");
    fprintf(outfile, "//  the Free Software Foundation; either version 2 of the License, or\n");
    fprintf(outfile, "//  (at your option) any later version.\n");
    fprintf(outfile, "//\n");
    fprintf(outfile, "//  This program is distributed in the hope that it will be useful,\n");
    fprintf(outfile, "//  but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    fprintf(outfile, "//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    fprintf(outfile, "//  GNU General Public License for more details.\n");
    fprintf(outfile, "//\n");
    fprintf(outfile, "//  You should have received a copy of the GNU General Public License\n");
    fprintf(outfile, "//  along with this program; if not, write to the Free Software\n");
    fprintf(outfile, "//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n");
    fprintf(outfile, "//  ---------------------------------------------------------------------------\n\n");
    fprintf(outfile, "{\n");
}

static void write_data(void)
{
    int i;

    counter = 0;

    while (counter < 4096) {
        fprintf(outfile, "/* 0x%03x: */ ", counter);
        for (i = 0; i < 8; ++i) {
            fprintf(outfile, " 0x%03x,", buffer[counter + i] << 4);
        }
        fprintf(outfile, "\n");
        counter += 8;
    }
    fprintf(outfile, "},\n");
}

int open_files(char *input_name, char *output_name)
{
    char real_input_name[256];
    char real_output_name[256];

    sprintf(real_input_name, "%s%s", inoutput_path, input_name);
    sprintf(real_output_name, "%s%s", inoutput_path, output_name);

    /* try to open both files */
    infile = fopen(real_input_name, "rb");
    if (infile == NULL) {
        printf("dat2h: failed to open '%s' for reading: %d: %s\n",
                real_input_name, errno, strerror(errno));
        return EXIT_FAILURE;
    }
    outfile = fopen(real_output_name, "wb");
    if (outfile == NULL) {
        printf("dat2h: failed to open '%s' for writing: %d: %s\n",
                real_output_name, errno, strerror(errno));
        fclose(infile);
        return EXIT_FAILURE;
    }

    if (fread(buffer, 1, 4096, infile) != 4096) {
        printf("dat2h: failed to read 4096 bytes from '%s'\n", real_input_name);
        fclose(infile);
        fclose(outfile);
        return EXIT_FAILURE;
    }    

    write_header();
    write_data();

    /* clean up */
    fclose(infile);
    fclose(outfile);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    int c = 0;
    int wrong = EXIT_SUCCESS;

    /* argument amount check */
    if (argc != 2) {
        printf("dat2h: wrong amount of arguments\n");
        return EXIT_FAILURE;
    }

    inoutput_path = argv[1];

    /* cycle through all the files */
    while (names[c].inname && wrong != EXIT_FAILURE) {
        wrong = open_files(names[c].inname, names[c].outname);
        ++c;
    }

    if (wrong == EXIT_SUCCESS) {
        printf("dat2h: conversion completed\n");
    } else {
        printf("dat2h: conversion failed\n");
    }

    return wrong;
}
