/*
 * getsize.c - File size helper program for the QNX package creation.
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

static void getsize(unsigned char *readbuffer, int filesize)
{
    int counter = 0;
    int sizecounter = 0;
    int foundspace = 0;
    int start = 0;
    int size = 0;
    int totalsize = 0;

    while (counter != filesize) {
        if (foundspace == 4) {
            if (isspace(readbuffer[counter])) {
                foundspace++;
            } else {
                foundspace = 0;
                size = 0;
            }
        }
        if (foundspace == 2 || foundspace == 3) {
            if (islower(readbuffer[counter])) {
                foundspace++;
            } else {
                foundspace = 0;
                size = 0;
            }
        }
        if (foundspace == 1) {
            if (isupper(readbuffer[counter])) {
                foundspace++;
            } else {
                foundspace = 0;
                size = 0;
            }
        }
        if (foundspace == 0) {
            if (isspace(readbuffer[counter])) {
                foundspace++;
            } else {
                if (isdigit(readbuffer[counter])) {
                    size = (size * 10) + readbuffer[counter] - '0';
                } else {
                    size = 0;
                }
            }
        }
        if (foundspace == 5) {
            totalsize = totalsize + size;
            foundspace = 0;
            size = 0;
        }
        counter++;
    }
    printf("%d\n", totalsize);
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
                            getsize(buffer, statbuf.st_size);
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
