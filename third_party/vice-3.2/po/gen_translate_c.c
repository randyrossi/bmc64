/*
 * gen_translate_c - Automatic tranlation system helper tool, this program will
 *                   regenerate the translate_text.h file with translations in place.
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

#define UNKNOWN              0
#define FOUND_EN             1
#define FOUND_END_OF_TABLE   2
#define FOUND_EOF            3
#define FOUND_EMPTY_LINE     4
#define FOUND_ENDIF          5
#define FOUND_IFDEF          6

static char line_buffer[4096];

static FILE *infile = NULL;
static FILE *outfile = NULL;

static int gentranslate_getline(FILE *file)
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
                    line_buffer[counter] = '\r';
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

    if (!strncasecmp(line_buffer, "#ifdef", 6)) {
        return FOUND_IFDEF;
    }

    if (!strncasecmp(line_buffer, "#endif", 6)) {
        return FOUND_ENDIF;
    }

    if (line_buffer[0] == '}' && line_buffer[1] == ';') {
        return FOUND_END_OF_TABLE;
    }

    if (!strncasecmp(line_buffer, "/* en */", 8)) {
        return FOUND_EN;
    }

    if (line_buffer[0] == '\n') {
        return FOUND_EMPTY_LINE;
    }

    if (line_buffer[0] == '\r' && line_buffer[1] == '\n') {
        return FOUND_EMPTY_LINE;
    }

    return UNKNOWN;
}

static void close_all(void)
{
    if (infile != NULL) {
        fclose(infile);
    }

    if (outfile != NULL) {
        fclose(outfile);
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

static void write_converted_text(char *text1, char *prefix_text, char *trail_text)
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
            text2[counter++] = '"';
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
    int i;
    int id_start;
    int text_start;
    char *id_string;
    char *text_string;
    char *trail_string;
    char *prefix_string;
    char *text_string_orig;

    infile = fopen("../src/translate_text.c", "rb");
    if (infile == NULL) {
        close_all();
        return 1;
    }

    outfile = fopen("../src/temp_translate.h", "wb");
    if (outfile == NULL) {
        close_all();
        return 1;
    }

    fprintf(outfile, "/*\n");
    fprintf(outfile, " * translate_text.c - Translation texts to be included in translate.c\n");
    fprintf(outfile, " *\n");
    fprintf(outfile, " * Written by\n");
    fprintf(outfile, " *  Marco van den Heuvel <blackystardust68@yahoo.com>\n");
    fprintf(outfile, " *\n");
    fprintf(outfile, " * This file is part of VICE, the Versatile Commodore Emulator.\n");
    fprintf(outfile, " * See README for copyright notice.\n");
    fprintf(outfile, " *\n");
    fprintf(outfile, " *  This program is free software; you can redistribute it and/or modify\n");
    fprintf(outfile, " *  it under the terms of the GNU General Public License as published by\n");
    fprintf(outfile, " *  the Free Software Foundation; either version 2 of the License, or\n");
    fprintf(outfile, " *  (at your option) any later version.\n");
    fprintf(outfile, " *\n");
    fprintf(outfile, " *  This program is distributed in the hope that it will be useful,\n");
    fprintf(outfile, " *  but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    fprintf(outfile, " *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    fprintf(outfile, " *  GNU General Public License for more details.\n");
    fprintf(outfile, " *\n");
    fprintf(outfile, " *  You should have received a copy of the GNU General Public License\n");
    fprintf(outfile, " *  along with this program; if not, write to the Free Software\n");
    fprintf(outfile, " *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA\n");
    fprintf(outfile, " *  02111-1307  USA.\n");
    fprintf(outfile, " *\n");
    fprintf(outfile, " */\n\n");
    fprintf(outfile, "translate_t string_table[] = {\n\n");
    while (found != FOUND_EMPTY_LINE) {
        found = gentranslate_getline(infile);
    }
    found = gentranslate_getline(infile);
    found = gentranslate_getline(infile);
    while (found != FOUND_END_OF_TABLE) {
        found = UNKNOWN;
        while (found != FOUND_EN && found != FOUND_END_OF_TABLE) {
            fwrite(line_buffer, 1, strlen(line_buffer), outfile);
            found = gentranslate_getline(infile);
        }
        if (found != FOUND_END_OF_TABLE) {
            i = 0;
            while (line_buffer[i] != '{') {
                i++;
            }
            id_start = i + 1;
            while (line_buffer[i] != ',') {
                i++;
            }
            line_buffer[i] = 0;
            while (line_buffer[i] != '"') {
                i++;
            }
            i++;
            text_start = i;
            while (!(line_buffer[i] == '"' && line_buffer[i + 1] == ')' && line_buffer[i + 2] == '}' && line_buffer[i + 3] == ',')) {
                i++;
            }
            line_buffer[i] = 0;
            id_string = strdup(line_buffer + id_start);
            text_string = strdup(line_buffer + text_start);
            fprintf(outfile, "/* en */ {%s,    N_(\"", line_buffer + id_start);
            write_converted_text(text_string, NULL, NULL);

            text_string_orig = strdup(text_string);

            prefix_string = prefix_test(text_string);
            trail_string = trailtest(text_string);

            fprintf(outfile, "\")},\n#ifdef HAS_TRANSLATION\n/* da */ {%s_DA, \"", id_string);
            for (i = 0; text[i].msgid != NULL; i++) {
                if (!strcmp(text[i].msgid, text_string)) {
                    break;
                }
                if (!strcmp(text[i].msgid, text_string_orig)) {
                    prefix_string = NULL;
                    trail_string = NULL;
                    break;
                }
            }
            if (text[i].msgid == NULL) {
                printf("Cannot find %s in the po-table\n", text_string);
                exit(1);
            }
            if (strlen(text[i].msgstr_da) != 0) {
                write_converted_text(text[i].msgstr_da, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_da) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* de */ {%s_DE, \"", id_string);

            if (strlen(text[i].msgstr_de) != 0) {
                write_converted_text(text[i].msgstr_de, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_de) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* es */ {%s_ES, \"", id_string);

            if (strlen(text[i].msgstr_es) != 0) {
                write_converted_text(text[i].msgstr_es, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_es) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* fr */ {%s_FR, \"", id_string);

            if (strlen(text[i].msgstr_fr) != 0) {
                write_converted_text(text[i].msgstr_fr, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_fr) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* hu */ {%s_HU, \"", id_string);

            if (strlen(text[i].msgstr_hu) != 0) {
                write_converted_text(text[i].msgstr_hu, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_hu) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* it */ {%s_IT, \"", id_string);

            if (strlen(text[i].msgstr_it) != 0) {
                write_converted_text(text[i].msgstr_it, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_it) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* ko */ {%s_KO, \"", id_string);

            if (strlen(text[i].msgstr_ko) != 0) {
                write_converted_text(text[i].msgstr_ko, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_ko) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* nl */ {%s_NL, \"", id_string);

            if (strlen(text[i].msgstr_nl) != 0) {
                write_converted_text(text[i].msgstr_nl, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_nl) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* pl */ {%s_PL, \"", id_string);

            if (strlen(text[i].msgstr_pl) != 0) {
                write_converted_text(text[i].msgstr_pl, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_pl) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* ru */ {%s_RU, \"", id_string);

            if (strlen(text[i].msgstr_ru) != 0) {
                write_converted_text(text[i].msgstr_ru, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_ru) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* sv */ {%s_SV, \"", id_string);

            if (strlen(text[i].msgstr_sv) != 0) {
                write_converted_text(text[i].msgstr_sv, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_sv) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n/* tr */ {%s_TR, \"", id_string);

            if (strlen(text[i].msgstr_tr) != 0) {
                write_converted_text(text[i].msgstr_tr, prefix_string, trail_string);
            }
            fprintf(outfile, "\"},");
            if (strlen(text[i].msgstr_tr) == 0) {
                fprintf(outfile, "  /* fuzzy */");
            }
            fprintf(outfile, "\n#endif\n");
            while (found != FOUND_ENDIF) {
                found = gentranslate_getline(infile);
            }
            found = gentranslate_getline(infile);
            if (found == FOUND_ENDIF) {
                fprintf(outfile, "#endif\n");
                found = gentranslate_getline(infile);
                if (found == FOUND_ENDIF) {
                    fprintf(outfile, "#endif\n");
                }
            }
            while (found != FOUND_EMPTY_LINE) {
                found = gentranslate_getline(infile);
            }
            free(id_string);
            free(text_string);
            free(text_string_orig);
        }
    }
    fprintf(outfile, "};\n");
    close_all();
    return 0;
}
