/*
 * getlibs.c - Library scanning helper program for the QNX package creation.
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
#include <sys/stat.h>

static void scan_libs(unsigned char *readbuffer, int filesize)
{
    int counter = 0;
    int libnamecounter = 0;
    int founddot = 0;
    int gotname = 0;
    int start = 0;
    char libname[200];

    while (counter != filesize) {
        if (libnamecounter > 2 && founddot == 4) {
            if (isdigit(readbuffer[counter])) {
                libname[libnamecounter] = readbuffer[counter];
                libnamecounter++;
            } else {
                libname[libnamecounter] = 0;
                gotname = 1;
            }
        }
        if (libnamecounter > 2 && founddot == 3) {
            if (readbuffer[counter] == '.') {
                libname[libnamecounter] = readbuffer[counter];
                libnamecounter++;
                founddot++;
            } else {
                libnamecounter = 0;
                founddot = 0;
            }
        }
        if (libnamecounter > 2 && founddot == 2) {
            if (readbuffer[counter] == 'o') {
                libname[libnamecounter] = readbuffer[counter];
                libnamecounter++;
                founddot++;
            } else {
                libnamecounter = 0;
                founddot=0;
            }
        }
        if (libnamecounter > 2 && founddot == 1) {
            if (readbuffer[counter] == 's') {
                libname[libnamecounter] = readbuffer[counter];
                libnamecounter++;
                founddot++;
            } else {
                libnamecounter = 0;
                founddot = 0;
            }
        }
        if (libnamecounter > 2 && founddot == 0) {
            if (isalpha(readbuffer[counter]) || isdigit(readbuffer[counter]) || readbuffer[counter] == '-' || readbuffer[counter] == '+' || readbuffer[counter] == '.') {
                if (readbuffer[counter] == '.' && readbuffer[counter + 1] == 's') {
                    founddot = 1;
                }
                libname[libnamecounter] = readbuffer[counter];
                libnamecounter++;
            } else {
                libnamecounter = 0;
            }
        }
        if (libnamecounter == 2) {
            if (readbuffer[counter] == 'b') {
                libname[libnamecounter] = readbuffer[counter];
                libnamecounter++;
            } else {
                libnamecounter = 0;
            }
        }
        if (libnamecounter == 1) {
            if (readbuffer[counter] == 'i') {
                libname[libnamecounter] = readbuffer[counter];
                libnamecounter++;
            } else {
                libnamecounter = 0;
            }
        }
        if (libnamecounter == 0) {
            if (readbuffer[counter] == 'l') {
                libname[libnamecounter] = readbuffer[counter];
                libnamecounter++;
                start = counter;
            }
        }
        counter++;
        if (gotname == 1) {
            printf("            <QPM:RequiresLibrary>%s</QPM:RequiresLibrary>\n", readbuffer + start);
            libnamecounter = 0;
            founddot = 0;
            libnamecounter = 0;
            gotname = 0;
        }
    }
}

int main(int argc, char **argv)
{
    struct stat statbuf;
    FILE *infile;
    unsigned char *buffer = NULL;

    if (argc == 2) {
        if (stat(argv[1], &statbuf) >= 0) {
            if (statbuf.st_size > 0) {
                buffer = (unsigned char*)malloc(statbuf.st_size);
                if (buffer != NULL) {
                    infile = fopen(argv[1], "rb");
                    if (infile) {
                        if (fread(buffer, 1, statbuf.st_size, infile) == statbuf.st_size) {
                            scan_libs(buffer, statbuf.st_size);
                        } else {
                            fclose(infile);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
