/*
 * intl2po - win32 intl resource and amiga intl.c to .po compatible
 *           format conversion helper program.
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
#include <stdlib.h>
#include <string.h>

#ifdef _SCO_ELF
#include <strings.h>
#endif

/* status definitions */
#define SCANNING                 0
#define STRINGTABLE_BEGIN_SCAN   1
#define MENU_BEGIN_SCAN          2
#define DIALOG_BEGIN_SCAN        3
#define TEXT_CONVERSION          4

/* found definitions */
#define UNKNOWN             0
#define FOUND_STRINGTABLE   1
#define FOUND_MENU          2
#define FOUND_DIALOG        3
#define FOUND_CAPTION       4
#define FOUND_BEGIN         5
#define FOUND_END           6

/* comment status */
#define NOT_IN_COMMENT   0
#define IN_COMMENT       1

static int comment_status=NOT_IN_COMMENT;
static int gettext_open_mark = 0;

static char line_buffer[512];

static char *convert_filename(char *name, char *src, char *dst)
{
    size_t src_len = strlen(src);
    size_t dst_len = strlen(dst);
    size_t name_len = strlen(name) - src_len + dst_len;
    char *real_name = NULL;
    size_t i = 0;

    /* sanity check */
    if (name_len < 1) {
        return strdup(name);
    }

    if (!strcmp(src, dst)) {
        return strdup(name);
    }

    real_name = malloc(name_len + 1);

    while (dst[i]) {
        real_name[i] = dst[i];
        i++;
    }

    while (name[src_len]) {
        real_name[i++] = name[src_len++];
    }
    real_name[i--] = 0;

    return real_name;
}


static int intl2po_getline(FILE *file)
{
    int c = 0;
    int counter = 0;

    while (c != '\n' && !feof(file) && counter < 511) {
        c = fgetc(file);
        line_buffer[counter] = (char)c;
        counter++;
    }
    line_buffer[counter] = 0;

    if (line_buffer[0] == '/' && line_buffer[1] == '/') {
        line_buffer[0] = 0;
        return UNKNOWN;
    }

    if (!strncasecmp(line_buffer, "STRINGTABLE", 11)) {
        return FOUND_STRINGTABLE;
    }

    if (!strncasecmp(line_buffer, "IDR_", 4)) {
        return FOUND_MENU;
    }

    if (!strncasecmp(line_buffer, "IDD_", 4)) {
        return FOUND_DIALOG;
    }

    if (!strncasecmp(line_buffer, "CAPTION", 7)) {
        return FOUND_CAPTION;
    }

    if (!strncasecmp(line_buffer, "BEGIN", 5)) {
        return FOUND_BEGIN;
    }

    if (!strncasecmp(line_buffer, "END", 3)) {
        return FOUND_END;
    }

    return UNKNOWN;
}


static void getline_simple(FILE *file)
{
    int c = 0;
    int counter = 0;

    while (c != '\n' && !feof(file) && counter < 511) {
        c = fgetc(file);
        line_buffer[counter] = (char)c;
        counter++;
    }
    line_buffer[counter] = 0;
}


static void check_gettext_end(char *text)
{
    int i;
    int first = 0;

    for (i = 0; text[i] != 0; i++) {
        if (text[i] == '"') {
            if (first == 1) {
                first = 0;
                if (text[i + 1] == ')') {
                    gettext_open_mark = 0;
                }
            } else {
                first = 1;
            }
        }
    }
}


static int check_quote(char *text)
{
    int i;

    if (text[0] == '/' && text[1] == '/') {
        return 0;
    }

    for (i = 0; text[i] != 0; i++) {
        if (text[i] == '"') {
            if (i - 3 >= 0) {
                if (text[i - 3] == 'N' && text[i - 2] == '_' && text[i - 1] == '(') {
                    gettext_open_mark = 1;
                    return 0;
                }
            }
            if (text[i + 1] != '"') {
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}


static int language_id(char *text)
{
    int i;

    for (i = 0; (text[i] != ' ' && text[i] != '\n'); i++) { }

    if (text[i] == '\n') {
        return 1;
    }

    if (!strncmp(text, "IDR", 3)) {
        if (strncmp(text + i + 1, "MENU", 4)) {
            return 1;
        }
    }

    if (!strncmp(text, "IDD", 3)) {
        if (strncmp(text + i + 1, "DIALOG" ,6)) {
            return 1;
        }
    }

    if (text[i - 3] == '_' && text[i - 2] == 'D' && text[i - 1] == 'A') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'D' && text[i - 1] == 'E') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'E' && text[i - 1] == 'S') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'F' && text[i - 1] == 'R') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'H' && text[i - 1] == 'U') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'I' && text[i - 1] == 'T') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'K' && text[i - 1] == 'O') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'N' && text[i - 1] == 'L') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'P' && text[i - 1] == 'L') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'R' && text[i - 1] == 'U') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'S' && text[i - 1] == 'V') {
        return 1;
    }

    if (text[i - 3] == '_' && text[i - 2] == 'T' && text[i - 1] == 'R') {
        return 1;
    }

    return 0;
}


static void remove_trailing_mascuerade(char *text)
{
    int i = 0;
    char *sub = NULL;

    sub = strstr(text, " (MasC=uerade)...\"");
    while (sub[i + 17] != 0) {
        sub[i] = sub[i + 17];
        i++;
    }
    sub[i] = 0;
}


static void remove_trailing_3_dots(char *text)
{
    int i = 0;
    char *sub = NULL;

    sub = strstr(text, "...\"");
    while (sub[i + 3] != 0) {
        sub[i] = sub[i + 3];
        i++;
    }
    sub[i] = 0;
}


static void remove_trailing_space_colon(char *text)
{
    int i = 0;
    char *sub = NULL;

    sub = strstr(text, " :\"");
    while (sub[i + 2] != 0) {
        sub[i] = sub[i + 2];
        i++;
    }
    sub[i] = 0;
}


static void remove_trailing_colon_space(char *text)
{
    int i = 0;
    char *sub = NULL;

    sub = strstr(text, ": \"");
    while (sub[i + 2] != 0) {
        sub[i] = sub[i + 2];
        i++;
    }
    sub[i] = 0;
}


static void remove_trailing_colon(char *text)
{
    int i = 0;
    char *sub = NULL;

    sub = strstr(text, ":\"");
    while (sub[i + 1] != 0) {
        sub[i] = sub[i + 1];
        i++;
    }
    sub[i] = 0;
}


static void remove_trailing_range(char *text)
{
    int i = 0;
    int j = 0;
    char *sub = NULL;

    sub = strstr(text, "..");

    while (sub[0] != '(') {
        sub--;
    }
    sub--;

    while (sub[i] != ')') {
        i++;
    }
    i++;

    while (sub[i] != 0) {
        sub[j++] = sub[i++];
    }
    sub[j] = 0;
}


static void remove_brackets(char *text)
{
    char *sub;
    int i = 2;
    int j = 1;

    sub = strstr(text, "\"<");

    while(sub[i] != '>') {
        sub[j++] = sub[i++];
    }
    i++;
    while (sub[i] != 0) {
        sub[j++] = sub[i++];
    }
    sub[j] = 0;
}


static void replace_string(char *text, FILE *file)
{
    int i, j;

    if (strstr(text, " (MasC=uerade)...\"") != NULL) {
        remove_trailing_mascuerade(text);
    }

    if (strstr(text, "...\"") != NULL) {
        remove_trailing_3_dots(text);
    }

    if (strstr(text, " :\"") != NULL) {
        remove_trailing_space_colon(text);
    }

    if (strstr(text, ": \"") != NULL) {
        remove_trailing_colon_space(text);
    }

    if (strstr(text, ":\"") != NULL) {
        remove_trailing_colon(text);
    }

    if (strstr(text, "..") != NULL) {
        remove_trailing_range(text);
    }

    if (strstr(text, "\"<") != NULL && strstr(text, ">\"") != NULL) {
        remove_brackets(text);
    }

    if (check_quote(text) == 0) {
        fprintf(file, "%s", text);
        if (gettext_open_mark) {
            check_gettext_end(text);
        }
    } else {
        for (i = 0; text[i] != '"'; i++) {
            fputc(text[i], file);
        }
        fputc('N', file);
        fputc('_', file);
        fputc('(', file);
        fputc(text[i], file);

        for (j = i + 1; !(text[j] == '"' && (text[j - 1] != '\\' || (text[j - 1] == '\\' && text[j - 2] == '\\'))); j++) {
            fputc(text[j], file);
        }
        fputc('"', file);
        fputc(')', file);

        for (i = j + 1; text[i] != 0; i++) {
            fputc(text[i], file);
        }
    }
}


static void wrong_location(char *text, FILE *infile, FILE *outfile, char *filename)
{
    printf("%s found at wrong location in %s\n", text, filename);
    fclose(infile);
    fclose(outfile);
}


static int convert_rc(char *in_filename, char *out_filename, char *src, char *dst)
{
    struct stat statbuf;
    FILE *infile, *outfile;
    int status = SCANNING;
    int found = UNKNOWN;
    int stringtable_found = 0;
    char *real_out_filename;

    if (stat(in_filename, &statbuf) < 0) {
        printf("cannot stat %s\n", in_filename);
        return 0;
    }

    if (statbuf.st_size == 0) {
        printf("file %s is 0 bytes\n", in_filename);
        return 0;
    }

    infile = fopen(in_filename, "rb");
    if (infile == NULL) {
        printf("cannot open %s for reading\n", in_filename);
        return 0;
    }

    real_out_filename = convert_filename(out_filename, src, dst);

    outfile = fopen(real_out_filename, "wb");
    if (outfile == NULL) {
        printf("cannot open %s for writing\n", real_out_filename);
        fclose(infile);
        free(real_out_filename);
        return 0;
    }

    while (!feof(infile)) {
        found = intl2po_getline(infile);
        switch(found) {
            case FOUND_STRINGTABLE:
                if (stringtable_found == 0) {
                    if (status != SCANNING) {
                        wrong_location("STRINGTABLE", infile, outfile, in_filename);
                        free(real_out_filename);
                        return 0;
                    }
                    status = STRINGTABLE_BEGIN_SCAN;
                    fprintf(outfile, "%s", line_buffer);
                } else {
                    fprintf(outfile, "%s", line_buffer);
                    intl2po_getline(infile);
                    if (!strncmp(line_buffer, "LANGUAGE LANG_ENGLISH", 21)) {
                        status = STRINGTABLE_BEGIN_SCAN;
                    }
                    fprintf(outfile, "%s", line_buffer);
                }
                break;
            case FOUND_MENU:
                if (status != SCANNING) {
                    wrong_location("MENU", infile, outfile, in_filename);
                    free(real_out_filename);
                    return 0;
                } else {
                    if (language_id(line_buffer) == 0) {
                        status = MENU_BEGIN_SCAN;
                    }
                    fprintf(outfile, "%s", line_buffer);
                }
                break;
            case FOUND_DIALOG:
                if (status != SCANNING) {
                    printf("%s", line_buffer);
                    wrong_location("DIALOG", infile, outfile, in_filename);
                    free(real_out_filename);
                    return 0;
                } else {
                    if (language_id(line_buffer) == 0) {
                        status = DIALOG_BEGIN_SCAN;
                    }
                    fprintf(outfile, "%s", line_buffer);
                }
                break;
            case FOUND_CAPTION:
                if (status == DIALOG_BEGIN_SCAN) {
                    replace_string(line_buffer, outfile);
                } else {
                    fprintf(outfile, "%s", line_buffer);
                }
                break;
            case FOUND_BEGIN:
                switch (status) {
                    case STRINGTABLE_BEGIN_SCAN:
                        stringtable_found = 1;
                        status = TEXT_CONVERSION;
                        break;
                    case MENU_BEGIN_SCAN:
                    case DIALOG_BEGIN_SCAN:
                        status = TEXT_CONVERSION;
                        break;
                    default:
                        break;
                }
                fprintf(outfile, "%s", line_buffer);
                break;
            case FOUND_END:
                status = SCANNING;
                fprintf(outfile, "%s", line_buffer);
                break;
            default:
                switch (status) {
                    case TEXT_CONVERSION:
                        replace_string(line_buffer, outfile);
                        break;
                    case SCANNING:
                        if (!feof(infile)) {
                            fprintf(outfile,"%s",line_buffer);
                        }
                        break;
                    default:
                        break;
                }
        }
    }
    fclose(infile);
    fclose(outfile);
    free(real_out_filename);
    return 1;
}


static void strip_comments(char *text)
{
    int i;

    for (i = 0; text[i] != 0; i++) {
        if (text[i] == '*' && text[i + 1] == '/') {
            text[i] = 32;
            text[i + 1] = 32;
            comment_status = NOT_IN_COMMENT;
        }
        if (comment_status == IN_COMMENT) {
            text[i] = 32;
        }
        if (text[i] == '/' && text[i + 1] == '*') {
            text[i]=32;
            comment_status = IN_COMMENT;
        }
    }
}


static int convert_intl(char *in_filename, char *out_filename, char *src, char *dst)
{
    struct stat statbuf;
    FILE *infile, *outfile;
    char *real_out_filename;

    if (stat(in_filename, &statbuf) < 0) {
        printf("cannot stat %s\n", in_filename);
        return 0;
    }

    if (statbuf.st_size == 0) {
        printf("file %s is 0 bytes\n", in_filename);
        return 0;
    }

    infile = fopen(in_filename, "rb");
    if (infile == NULL) {
        printf("cannot open %s for reading\n", in_filename);
        return 0;
    }

    real_out_filename = convert_filename(out_filename, src, dst);

    outfile = fopen(real_out_filename, "wb");
    if (outfile == NULL) {
        printf("cannot open %s for writing\n", real_out_filename);
        free(real_out_filename);
        fclose(infile);
        return 0;
    }

    while (!feof(infile)) {
        getline_simple(infile);
        if (!feof(infile)) {
            if (!strncmp(line_buffer, "/* en */" ,8)) {
                strip_comments(line_buffer);
                replace_string(line_buffer,outfile);
            } else {
                strip_comments(line_buffer);
                fprintf(outfile,"%s",line_buffer);
            }
        }
    }
    fclose(infile);
    fclose(outfile);
    free(real_out_filename);
    return 1;
}



int main(int argc, char *argv[])
{
    int result = 0;

    if (argc < 6) {
        printf("too few arguments\n");
        exit(1);
    }

    if (!strcasecmp(argv[1], "win32") && argc == 6) {
        result = convert_rc(argv[2], argv[3], argv[4], argv[5]);
    }

    if (!strcasecmp(argv[1], "intl") && argc == 6) {
        result = convert_intl(argv[2], argv[3], argv[4], argv[5]);
    }

    if (result == 0) {
        printf("error while processing file\n");
        exit(1);
    }
    return 0;
}
