/*
 * gen_win32_c - Automatic tranlation system helper tool, this program will
 *               regenerate the provided .rc file with translations in place.
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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef _SCO_ELF
#include <strings.h>
#endif

#include "po_table.h"

#define UNKNOWN             0
#define FOUND_STRINGTABLE   1
#define FOUND_BEGIN         2
#define FOUND_END           3
#define FOUND_IFDEF         4
#define FOUND_ENDIF         5
#define FOUND_EOF           6
#define FOUND_ELSE          7

static char line_buffer[4096];

typedef struct rc_file_table_s {
    char *filename;
    FILE *filehandle;
    char *title;
    char *lang;
    char *sublang;
    char *pragma;
} rc_file_table_t;

static rc_file_table_t files[] = {
    { "../src/arch/win32/temp_en.rc", NULL, NULL, NULL, NULL, NULL },
    { "../src/arch/win32/temp_da.rc", NULL, "// DANISH text", "LANG_DANISH", "SUBLANG_NEUTRAL", NULL },
    { "../src/arch/win32/temp_de.rc", NULL, "// GERMAN text", "LANG_GERMAN", "SUBLANG_NEUTRAL", NULL },
    { "../src/arch/win32/temp_es.rc", NULL, "// SPANISH text", "LANG_SPANISH", "SUBLANG_NEUTRAL", NULL },
    { "../src/arch/win32/temp_fr.rc", NULL, "// FRENCH text", "LANG_FRENCH", "SUBLANG_NEUTRAL", NULL },
    { "../src/arch/win32/temp_hu.rc", NULL, "// HUNGARIAN text", "LANG_HUNGARIAN", "SUBLANG_NEUTRAL", "28592" },
    { "../src/arch/win32/temp_it.rc", NULL, "// ITALIAN text", "LANG_ITALIAN", "SUBLANG_NEUTRAL", NULL },
    { "../src/arch/win32/temp_ko.rc", NULL, "// KOREAN text", "LANG_KOREAN", "SUBLANG_NEUTRAL", "949" },
    { "../src/arch/win32/temp_nl.rc", NULL, "// DUTCH text", "LANG_DUTCH", "SUBLANG_NEUTRAL", NULL },
    { "../src/arch/win32/temp_pl.rc", NULL, "// POLISH text", "LANG_POLISH", "SUBLANG_NEUTRAL", "28592" },
    { "../src/arch/win32/temp_ru.rc", NULL, "// RUSSIAN text", "LANG_RUSSIAN", "SUBLANG_NEUTRAL", "28595" },
    { "../src/arch/win32/temp_sv.rc", NULL, "// SWEDISH text", "LANG_SWEDISH", "SUBLANG_NEUTRAL", NULL },
    { "../src/arch/win32/temp_tr.rc", NULL, "// TURKISH text", "LANG_TURKISH", "SUBLANG_NEUTRAL", "28599" },
    { NULL, NULL, NULL, NULL, NULL, NULL }
};

static FILE *infile = NULL;

static int genrc_getline(FILE *file)
{
    char c = 0;
    int counter = 0;

    while (c != '\n' && !feof(file) && counter < 4095) {
        c = fgetc(file);
        line_buffer[counter] = c;
        if (line_buffer[counter] == '\\') {
            c = fgetc(file);
            switch (c) {
                case 'n':
                    line_buffer[counter] = '\n';
                    break;
                case 'r':
                    counter--;
                    break;
                case 't':
                    line_buffer[counter] = '\t';
                    break;
                case '"':
                    line_buffer[counter] = '"';
                    break;
                case '\\':
                    line_buffer[counter] = '\\';
                    break;
            }
        }
        counter++;
    }
    line_buffer[counter] = 0;

    if (feof(file)) {
        return FOUND_EOF;
    }

    if (!strncasecmp(line_buffer, "STRINGTABLE", 11)) {
        return FOUND_STRINGTABLE;
    }

    if (!strncasecmp(line_buffer, "BEGIN", 5)) {
        return FOUND_BEGIN;
    }

    if (!strncasecmp(line_buffer, "END", 3)) {
        return FOUND_END;
    }

    if (!strncasecmp(line_buffer, "#ifdef", 6)) {
        return FOUND_IFDEF;
    }

    if (!strncasecmp(line_buffer, "#else", 5)) {
        return FOUND_ELSE;
    }

    if (!strncasecmp(line_buffer, "#endif", 6)) {
        return FOUND_ENDIF;
    }

    return UNKNOWN;
}

static void close_all(void)
{
    int i;

    for (i = 0; files[i].filename != NULL; i++) {
        if (files[i].filehandle != NULL) {
            fclose(files[i].filehandle);
        }
    }

    if (infile != NULL) {
        fclose(infile);
    }
}

static char *trailtest(char *text)
{
    int i = 1;
    int j = 0;
    char *sub = NULL;

    sub = strstr(text, " (MasC=uerade)...\0");
    if (sub != NULL) {
        sub[0] = 0;
        return " (MasC=uerade)...";
    }

    sub = strstr(text, "...\0");
    if (sub != NULL) {
        sub[0] = 0;
        return "...";
    }

    sub = strstr(text, " :\0");
    if (sub != NULL) {
        sub[0] = 0;
        return " :";
    }

    sub = strstr(text, ": \0");
    if (sub != NULL) {
        sub[0] = 0;
        return ": ";
    }

    sub = strstr(text, ":\0");
    if (sub != NULL) {
        sub[0] = 0;
        return ":";
    }

    sub = strstr(text, "..");
    if (sub != NULL) {
        while (sub[0] != '(') {
            sub--;
        }
        sub--;
        
        sub[0] = 0;
        return sub + 1;
    }

    if (text[0] == '<' && strstr(text, ">\0") != NULL) {
        while (text[i] != '>') {
            text[j++] = text[i++];
        }
        text[j] = 0;
        return ">";
    }

    return NULL;
}

static char *prefix_test(char *text)
{
    if (text[0] == '<' && strstr(text, ">\0") != NULL) {
        return "<";
    }

    return NULL;
}

static void write_converted_text(char *text1, FILE *outfile, char *prefix_text, char *trail_text)
{
    char *text2 = malloc(strlen(text1) * 4);
    int i;
    int counter = 0;

    if (prefix_text != NULL) {
        for (i = 0; prefix_text[i] != 0; i++) {
            text2[counter++] = prefix_text[i];
        }
    }
    for (i = 0; text1[i] != 0; i++) {
        if (text1[i] == '"') {
            text2[counter++] = '\\';
            text2[counter++] = '0';
            text2[counter++] = '4';
            text2[counter++] = '2';
        } else if (text1[i] == '\n') {
            text2[counter++] = '\\';
            text2[counter++] = 'n';
        } else if (text1[i] == '\r') {
            text2[counter++] = '\\';
            text2[counter++] = 'r';
        } else {
            text2[counter++] = text1[i];
        }
    }
    if (trail_text != NULL) {
        if (trail_text[0] == '(') {
            text2[counter++] = ' ';
        }
        for (i = 0; trail_text[i] != 0; i++) {
            text2[counter++] = trail_text[i];
        }
    }
    text2[counter] = 0;
    fwrite(text2, 1, strlen(text2), outfile);
    free(text2);
}

int main(int argc, char *argv[])
{
    int found = UNKNOWN;
    int i = 0;
    int id;
    int failed = 0;
    char *text_string;
    char *prefix_string;
    char *trail_string;
    char *text_string_orig;

    if (argc != 2) {
        return 1;
    }

    infile = fopen(argv[1], "rb");
    if (infile == NULL) {
        close_all();
        return 1;
    }

    for (i = 0; files[i].filename != NULL; i++) {
        files[i].filehandle = fopen(files[i].filename, "wb");
        if (files[i].filehandle == NULL) {
            failed = 1;
        }
    }

    if (failed == 1) {
        close_all();
        return 1;
    }

    while (found != FOUND_STRINGTABLE && found != FOUND_EOF) {
        found = genrc_getline(infile);
        if (found != FOUND_EOF) {
            fwrite(line_buffer, 1, strlen(line_buffer), files[0].filehandle);
        }
    }
    if (found != FOUND_EOF) {
        for (i = 1; files[i].filename != NULL; i++) {
            fprintf(files[i].filehandle, "%s\n\n", files[i].title);
            if (files[i].pragma != NULL) {
                fprintf(files[i].filehandle, "#ifndef WINDRES_CP_IGNORE\n");
                fprintf(files[i].filehandle, "#pragma code_page(%s)\n", files[i].pragma);
                fprintf(files[i].filehandle, "#endif\n");
            }
            fprintf(files[i].filehandle, "STRINGTABLE\n");
            fprintf(files[i].filehandle, "LANGUAGE %s, %s\n", files[i].lang, files[i].sublang);
            fprintf(files[i].filehandle, "BEGIN\n");
        }
        while (found != FOUND_BEGIN) {
            found = genrc_getline(infile);
            fwrite(line_buffer, 1, strlen(line_buffer), files[0].filehandle);
        }
        found = genrc_getline(infile);
        while (found != FOUND_END) {
            if (found == FOUND_IFDEF || found == FOUND_ENDIF || found == FOUND_ELSE) {
                for (i = 0; files[i].filename != NULL; i++) {
                    fwrite(line_buffer, 1, strlen(line_buffer), files[i].filehandle);
                }
            } else {
                i = 0;
                while (line_buffer[i] != '"') {
                    i++;
                }
                line_buffer[i++] = 0;
                text_string = line_buffer + i;
                i += strlen(text_string);
                while (line_buffer[i] != '"') {
                    i--;
                }
                line_buffer[i] = 0;
                fprintf(files[0].filehandle, "%s\"", line_buffer);
                write_converted_text(text_string, files[0].filehandle, NULL, NULL);
                fprintf(files[0].filehandle, "\"\n");

                text_string_orig = strdup(text_string);

                prefix_string = prefix_test(text_string);
                trail_string = trailtest(text_string);

                for (id = 0; text[id].msgid != NULL; id++) {
                    if (!strcmp(text[id].msgid, text_string)) {
                        break;
                    }
                    if (!strcmp(text[id].msgid, text_string_orig)) {
                        prefix_string = NULL;
                        trail_string = NULL;
                        break;
                    }
                }
                for (i = 1; files[i].filename != NULL; i++) {
                    fprintf(files[i].filehandle, "%s\"", line_buffer);
                    switch (i) {
                        case 1:
                            if (strlen(text[id].msgstr_da) != 0) {
                                write_converted_text(text[id].msgstr_da, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 2:
                            if (strlen(text[id].msgstr_de) != 0) {
                                write_converted_text(text[id].msgstr_de, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 3:
                            if (strlen(text[id].msgstr_es) != 0) {
                                write_converted_text(text[id].msgstr_es, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 4:
                            if (strlen(text[id].msgstr_fr) != 0) {
                                write_converted_text(text[id].msgstr_fr, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 5:
                            if (strlen(text[id].msgstr_hu) != 0) {
                                write_converted_text(text[id].msgstr_hu, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 6:
                            if (strlen(text[id].msgstr_it) != 0) {
                                write_converted_text(text[id].msgstr_it, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 7:
                            if (strlen(text[id].msgstr_ko) != 0) {
                                write_converted_text(text[id].msgstr_ko, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 8:
                            if (strlen(text[id].msgstr_nl) != 0) {
                                write_converted_text(text[id].msgstr_nl, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 9:
                            if (strlen(text[id].msgstr_pl) != 0) {
                                write_converted_text(text[id].msgstr_pl, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 10:
                            if (strlen(text[id].msgstr_ru) != 0) {
                                write_converted_text(text[id].msgstr_ru, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 11:
                            if (strlen(text[id].msgstr_sv) != 0) {
                                write_converted_text(text[id].msgstr_sv, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                        case 12:
                            if (strlen(text[id].msgstr_tr) != 0) {
                                write_converted_text(text[id].msgstr_tr, files[i].filehandle, prefix_string, trail_string);
                            } else {
                                write_converted_text(text_string, files[i].filehandle, prefix_string, trail_string);
                            }
                            break;
                    }
                    fprintf(files[i].filehandle, "\"\n");
                }
                free(text_string_orig);
            }
            found = genrc_getline(infile);
        }
        for (i = 0; files[i].filename != NULL; i++) {
            fprintf(files[i].filehandle, "END\n");
            if (files[i].pragma != NULL) {
                fprintf(files[i].filehandle, "#ifndef WINDRES_CP_IGNORE\n");
                fprintf(files[i].filehandle, "#pragma code_page(28591)\n");
                fprintf(files[i].filehandle, "#endif\n");
            }
            fprintf(files[i].filehandle, "\n\n");
        }
    }
    close_all();
    return 0;
}
