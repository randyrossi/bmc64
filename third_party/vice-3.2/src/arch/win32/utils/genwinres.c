#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>

int idr_counter;
int idd_counter;
int idc_counter;
int ids_counter;
int idm_counter;
int idi_counter;
int general_counter;

int idr_start;
int idd_start;
int idc_start;
int ids_start;
int idm_start;
int idi_start;
int general_start;

unsigned char buffer[4096];
int buffer_size;
int buffer_readpointer;
unsigned char header[3];
char header_index;

void read_buffer(FILE *file)
{
    buffer_size = fread(buffer, 1, 4096, file);
    buffer_readpointer = 0;
}

#define GET_HEADER_FIRST_BYTE  0
#define GET_HEADER_SECOND_BYTE 1
#define GET_HEADER_THIRD_BYTE  2
#define FIND_LINE_END          3
#define SKIP_LINE              4
#define OUTPUT_LINE            5

void process_file(char *filename, FILE *houtput, FILE *moutput, FILE *soutput)
{
    FILE *infile;
    int state;
    int res_value = 0;
    int is_idm = 0;
    int is_ids = 0;

    infile = fopen(filename, "rb");
    if (infile) {
        state = GET_HEADER_FIRST_BYTE;
        read_buffer(infile);
        while (buffer_size) {
            buffer_readpointer = 0;
            while (buffer_readpointer < buffer_size) {
                switch (state) {
                    case GET_HEADER_FIRST_BYTE:
                        is_idm = 0;
                        is_ids = 0;
                        header[0] = buffer[buffer_readpointer];
                        if ((header[0] == 0x0a) || (header[1] == 0x0d)) {
                            // Empty line, find start of next line.
                            state = SKIP_LINE;
                        } else if ((header[0] == ' ') || (header[0] == 0x09)) {
                            // skip leading whitespace chars
                        } else {
                            state = GET_HEADER_SECOND_BYTE;
                        }
                        buffer_readpointer++;
                        break;
                    case GET_HEADER_SECOND_BYTE:
                        header[1] = buffer[buffer_readpointer];
                        if ((header[1] == 0x0a) || (header[1] == 0x0d)) {
                            //  Broken line skip it.
                            state = SKIP_LINE;
                        } else if ((header[0] == '/') && (header[1] == '/')) {
                            //  Comment, skip the line
                            state = FIND_LINE_END;
                        } else {
                            state = GET_HEADER_THIRD_BYTE;
                        }
                        buffer_readpointer++;
                        break;
                    case GET_HEADER_THIRD_BYTE:
                        header[2] = buffer[buffer_readpointer];
                        if ((header[2] == 0x0a) || (header[2] == 0x0d)) {
                            //  Broken line skip it.
                            state = SKIP_LINE;
                        } else {
                            if ((header[0] == 'I') && (header[1] == 'D') && (header[2] == 'R')) {
                                res_value = idr_counter++;
                                fprintf(houtput, "#define ");
                                fwrite(header, 1, 3, houtput);
                                state = OUTPUT_LINE;
                            } else if ((header[0] == 'I') && (header[1] == 'D') && (header[2] == 'D')) {
                                res_value = idd_counter++;
                                fprintf(houtput, "#define ");
                                fwrite(header, 1, 3, houtput);
                                state = OUTPUT_LINE;
                            } else if ((header[0] == 'I') && (header[1] == 'D') && (header[2] == 'C')) {
                                res_value = idc_counter++;
                                fprintf(houtput, "#define ");
                                fwrite(header, 1, 3, houtput);
                                state = OUTPUT_LINE;
                            } else if ((header[0] == 'I') && (header[1] == 'D') && (header[2] == 'S')) {
                                res_value = ids_counter++;
                                is_ids = 1;
                                fprintf(houtput, "#define ");
                                fwrite(header, 1, 3, houtput);
                                fprintf(soutput, "  { \"");
                                fwrite(header, 1, 3, soutput);
                                state = OUTPUT_LINE;
                            } else if ((header[0] == 'I') && (header[1] == 'D') && (header[2] == 'M')) {
                                res_value = idm_counter++;
                                is_idm = 1;
                                fprintf(houtput, "#define ");
                                fwrite(header, 1, 3, houtput);
                                fprintf(moutput, "  { \"");
                                fwrite(header, 1, 3, moutput);
                                state = OUTPUT_LINE;
                            } else if ((header[0] == 'I') && (header[1] == 'D') && (header[2] == 'I')) {
                                res_value = idi_counter++;
                                fprintf(houtput, "#define ");
                                fwrite(header, 1, 3, houtput);
                                state = OUTPUT_LINE;
                            } else {
                                res_value = general_counter++;
                                fprintf(houtput, "#define ");
                                fwrite(header, 1, 3, houtput);
                                state = OUTPUT_LINE;
                            }
                        }
                        buffer_readpointer++;
                        break;
                    case FIND_LINE_END:
                        if ((buffer[buffer_readpointer] == 0x0a) || (buffer[buffer_readpointer] == 0x0d)) {
                            state = SKIP_LINE;
                        }
                        buffer_readpointer++;
                        break;
                    case SKIP_LINE:
                        if ((buffer[buffer_readpointer] != 0x0a) && (buffer[buffer_readpointer] != 0x0d)) {
                            state = GET_HEADER_FIRST_BYTE;
                        } else {
                            buffer_readpointer++;
                        }
                        break;
                    case OUTPUT_LINE:
                        if ((buffer[buffer_readpointer] == 0x0a) || (buffer[buffer_readpointer] == 0x0d)) {
                            fprintf(houtput, " %d\n", res_value);
                            if (is_idm) {
                                fprintf(moutput, "\", %d }, \n", res_value);
                            }
                            if (is_ids) {
                                fprintf(soutput, "\", %d }, \n", res_value);
                            }
                            state = GET_HEADER_FIRST_BYTE;
                        } else {
                            fwrite(&buffer[buffer_readpointer], 1, 1, houtput);
                            if (is_idm) {
                                fwrite(&buffer[buffer_readpointer], 1, 1, moutput);
                            }
                            if (is_ids) {
                                fwrite(&buffer[buffer_readpointer], 1, 1, soutput);
                            }
                        }
                        buffer_readpointer++;
                        break;
                }
            }
            read_buffer(infile);
        }
        fclose(infile);
    }
}

int main(int argc, char **argv)
{
    FILE *houtput, *moutput, *soutput;
    int i;

    if (argc < 5) {
        printf("Usage: genwinres header-houtput menuid-houtput stringid-houtput source-filename [source-filenames]\n");
        exit(0);
    }
    houtput = fopen(argv[1], "wt");
    moutput = fopen(argv[2], "wt");
    soutput = fopen(argv[3], "wt");
    if (houtput && moutput && soutput) {
        idr_start = idr_counter = 100;
        idd_start = idd_counter = 100;
        /* Command ID codes 0-11 and 32000 are used by the OS as well, to be safe we start at 100 */
        idc_start = idc_counter = 100;
        ids_start = ids_counter = 1;
        idm_start = idm_counter = 100;
        idi_start = idi_counter = 1;
        general_start = general_counter = 1;

        fprintf(houtput, "/*\n");
        fprintf(houtput, " * %s\n", argv[1]);
        fprintf(houtput, " *\n");
        fprintf(houtput, " * Autogenerated genwinres file, DO NOT EDIT !!!\n");
        fprintf(houtput, " *\n");
        fprintf(houtput, " * Written by\n");
        fprintf(houtput, " *  Andreas Boose <viceteam@t-online.de>\n");
        fprintf(houtput, " *  Ettore Perazzoli <ettore@comm2000.it>\n");
        fprintf(houtput, " *  Tibor Biczo <crown@t-online.hu>\n");
        fprintf(houtput, " *  Marco van den Heuvel <blackystardust68@yahoo.com>\n");
        fprintf(houtput, " *\n");
        fprintf(houtput, " * This file is part of VICE, the Versatile Commodore Emulator.\n");
        fprintf(houtput, " * See README for copyright notice.\n");
        fprintf(houtput, " *\n");
        fprintf(houtput, " *  This program is free software; you can redistribute it and/or modify\n");
        fprintf(houtput, " *  it under the terms of the GNU General Public License as published by\n");
        fprintf(houtput, " *  the Free Software Foundation; either version 2 of the License, or\n");
        fprintf(houtput, " *  (at your option) any later version.\n");
        fprintf(houtput, " *\n");
        fprintf(houtput, " *  This program is distributed in the hope that it will be useful,\n");
        fprintf(houtput, " *  but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
        fprintf(houtput, " *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
        fprintf(houtput, " *  GNU General Public License for more details.\n");
        fprintf(houtput, " *\n");
        fprintf(houtput, " *  You should have received a copy of the GNU General Public License\n");
        fprintf(houtput, " *  along with this program; if not, write to the Free Software\n");
        fprintf(houtput, " *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA\n");
        fprintf(houtput, " *  02111-1307  USA.\n");
        fprintf(houtput, " *\n");
        fprintf(houtput, " */\n");
        fprintf(houtput, "\n");
        fprintf(houtput, "#ifndef VICE_RES_H\n");
        fprintf(houtput, "#define VICE_RES_H\n");
        fprintf(houtput, "\n");

        fprintf(moutput, "/*\n");
        fprintf(moutput, " * %s\n", argv[2]);
        fprintf(moutput, " *\n");
        fprintf(moutput, " * Autogenerated genwinres file, DO NOT EDIT !!!\n");
        fprintf(moutput, " *\n");
        fprintf(moutput, " */\n");
        fprintf(moutput, "#ifndef VICE_MENUID_H\n");
        fprintf(moutput, "#define VICE_MENUID_H\n");
        fprintf(moutput, "\n");
        fprintf(moutput, "static struct { char *str; int cmd; } idmlist[] = {\n");

        fprintf(soutput, "/*\n");
        fprintf(soutput, " * %s\n", argv[2]);
        fprintf(soutput, " *\n");
        fprintf(soutput, " * Autogenerated genwinres file, DO NOT EDIT !!!\n");
        fprintf(soutput, " *\n");
        fprintf(soutput, " */\n");
        fprintf(soutput, "#ifndef VICE_STRINGID_H\n");
        fprintf(soutput, "#define VICE_STRINGID_H\n");
        fprintf(soutput, "\n");
        fprintf(soutput, "static struct { char *str; int cmd; } idslist[] = {\n");

        for (i = 4; i < argc; i++) {
            fprintf(houtput, "/*  Definitions from %s  */\n\n", argv[i]);
            process_file(argv[i], houtput, moutput, soutput);
            fprintf(houtput, "\n");
        }
        fprintf(houtput, "#define FIRST_IDR %d\n", idr_start);
        fprintf(houtput, "#define LAST_IDR %d\n", idr_counter);
        fprintf(houtput, "#define FIRST_IDD %d\n", idd_start);
        fprintf(houtput, "#define LAST_IDD %d\n", idd_counter);
        fprintf(houtput, "#define FIRST_IDC %d\n", idc_start);
        fprintf(houtput, "#define LAST_IDC %d\n", idc_counter);
        fprintf(houtput, "#define FIRST_IDS %d\n", ids_start);
        fprintf(houtput, "#define LAST_IDS %d\n", ids_counter);
        fprintf(houtput, "#define FIRST_IDM %d\n", idm_start);
        fprintf(houtput, "#define LAST_IDM %d\n", idm_counter);
        fprintf(houtput, "#define FIRST_IDI %d\n", idi_start);
        fprintf(houtput, "#define LAST_IDI %d\n", idi_counter);
        fprintf(houtput, "#define FIRST_GENERAL %d\n", general_start);
        fprintf(houtput, "#define LAST_GENERAL %d\n", general_counter);
        fprintf(houtput, "\n");
        fprintf(houtput, "#endif\n");

        fprintf(moutput, " { NULL, 0 }\n};\n");
        fprintf(moutput, "#endif\n");

        fprintf(soutput, " { NULL, 0 }\n};\n\n");
        fprintf(soutput, "#define LAST_IDS %d\n\n", ids_counter);
        fprintf(soutput, "#endif\n");
    }
    if (houtput) {
        fclose(houtput);
    }
    if (moutput) {
        fclose(moutput);
    }
    if (soutput) {
        fclose(soutput);
    }

    return 0;
}
