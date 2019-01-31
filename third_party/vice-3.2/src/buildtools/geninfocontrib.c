/*
 * geninfocontrib - infocontrib.h generation helper program.
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

#define _CRT_SECURE_NO_DEPRECATE

#ifdef _minix_vmd
#define _MINIX_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#ifdef NEXT
#define NEED_STRDUP
#endif

#ifdef ultrix
#define NEED_STRDUP
#endif

#ifdef NEED_STRDUP
static char *strdup(const char *string)
{
    char *new;

    new = malloc(strlen(string) + 1);
    if (new != NULL) {
        strcpy(new, string);
    }
    return new;
}
#endif

static char line_buffer[512];
static char text[65536];

static char *name[256];
static char *emailname[256];

int get_line(FILE *file)
{
    int c = 0;
    int counter = 0;

    while (c != '\n' && !feof(file) && counter < 511) {
        c = fgetc(file);
        if (c != 0xd) {
            line_buffer[counter++] = (char)c;
        }
    }
    if (counter > 1) {
        line_buffer[counter - 1] = 0;
    }

    if (feof(file)) {
        return -1;
    }

    return counter - 1;
}

static void read_sed_file(FILE *sedfile)
{
    int amount = 0;
    int counter = 0;
    int foundend = 0;
    int buffersize = 0;

    buffersize = fread(text, 1, 65536, sedfile);
    text[buffersize] = 0;

    while (foundend == 0) {
        if (text[counter] == 0) {
            foundend = 1;
        } else {
            if (text[counter] == 's' && text[counter + 1] == '/' && text[counter + 2] == '@' && text[counter + 3] == 'b') {
                while (text[counter++] != '{') {}
                name[amount] = text + counter;
                while (text[counter++] != '}') {}
                text[counter - 1] = 0;
                counter++;
                emailname[amount++] = text + counter;
                while (text[counter++] != '/') {}
                text[counter - 1] = 0;
            }
            while (text[counter++] != '\n') {}
        }
    }
    name[amount] = NULL;
    emailname[amount] = NULL;
}

static int checklineignore(void)
{
    if (!strncmp(line_buffer, "@c", 2)) {
        return 1;
    }
    if (!strncmp(line_buffer, "@itemize @bullet", 16)) {
        return 1;
    }
    if (!strncmp(line_buffer, "@item", 5)) {
        return 1;
    }
    if (!strncmp(line_buffer, "@end itemize", 12)) {
        return 1;
    }
    return 0;
}

static void replacetags(void)
{
#ifdef _MSC_VER
    char *temp = _strdup(line_buffer);
#else
    char *temp = strdup(line_buffer);
#endif
    int countersrc = 0;
    int counterdst = 0;
    int i, j, len;
    int foundname;

    while (temp[countersrc] != 0) {
        if (temp[countersrc] == '@') {
            countersrc++;
            if (temp[countersrc]  == '@') {
                countersrc++;
                line_buffer[counterdst++] = '@';
                line_buffer[counterdst++] = '@';
            } else if (!strncmp(temp + countersrc, "b{", 2)) {
                countersrc += 2;
                foundname = 0;
                for (i = 0; name[i] != NULL && !foundname; i++) {
                    if (!strncmp(temp + countersrc, name[i], strlen(name[i]))) {
                        len = strlen(emailname[i]);
                        for (j = 0; j < len; j++) {
                            line_buffer[counterdst++] = emailname[i][j];
                        }
                    foundname = 1;
                    }
                }
                while (temp[countersrc++] != '}') {}
            } else if (!strncmp(temp + countersrc, "t{", 2)) {
                countersrc += 2;
                while (temp[countersrc] != '}') {
                    line_buffer[counterdst++] = temp[countersrc++];
                }
                countersrc++;
            } else if (!strncmp(temp + countersrc, "code{", 5)) {
                countersrc += 5;
                line_buffer[counterdst++] = '`';
                while (temp[countersrc] != '}') {
                    line_buffer[counterdst++] = temp[countersrc++];
                }
                countersrc++;
                line_buffer[counterdst++] = '\'';
            } else if (!strncmp(temp + countersrc, "dots{", 5)) {
                countersrc += 6;
                line_buffer[counterdst++] = '.';
                line_buffer[counterdst++] = '.';
                line_buffer[counterdst++] = '.';
                line_buffer[counterdst++] = '.';
            } else {
                countersrc += 11;
                line_buffer[counterdst++] = '(';
                line_buffer[counterdst++] = 'C';
                line_buffer[counterdst++] = ')';
            }
        } else {
            line_buffer[counterdst++] = temp[countersrc++];
        }
    }
    line_buffer[counterdst] = 0;
    free(temp);
}

static void strip_name_slashes(char *text)
{
    int i = 0;
    int j = 0;

    while (text[i] != 0) {
        if (text[i] == '\\') {
            i++;
        } else {
            text[j++] = text[i++];
        }
    }
    text[j] = 0;
}

static void strip_emailname_slashes(char *text)
{
    int i = 0;
    int j = 0;

    while (text[i] != 0) {
        if (text[i] == '\\' && text[i + 1] != '"') {
            i++;
        } else {
            text[j++] = text[i++];
        }
    }
    text[j] = 0;
}

static void replace_tokens(void)
{
    int i = 0;
    char *found = NULL;

    while (name[i] != NULL) {
        found = strstr(name[i], "\\");
        if (found != NULL) {
            strip_name_slashes(name[i]);
            strip_emailname_slashes(emailname[i]);
        }
        i++;
    }
}

static char *vice_stralloc(const char *str)
{
    size_t size;
    char *ptr;

    if (str == NULL) {
        exit(-1);
    }

    size = strlen(str) + 1;
    ptr = malloc(size);

    memcpy(ptr, str, size);

    return ptr;
}

static char *core_team[200];
static char *ex_team[200];
static char *trans_team[200];
static char *doc_team[100];

static char *all_team[300];

static int is_in_all_team_list(char *name)
{
    int i;

    for (i = 0; all_team[i]; i++) {
        if (!strcmp(name, all_team[i])) {
            return 1;
        }
    }
    return 0;
}

static void fill_all_team_list(void)
{
    int count = 0;
    int i = 0;

    all_team[count] = NULL;

    while (core_team[i] != NULL) {
        if (is_in_all_team_list(core_team[i + 1]) == 0) {
            all_team[count++] = core_team[i + 1];
            all_team[count] = NULL;
        }
        i += 2;
    }

    i = 0;
    while (ex_team[i] != NULL) {
        if (is_in_all_team_list(ex_team[i + 1]) == 0) {
            all_team[count++] = ex_team[i + 1];
            all_team[count] = NULL;
        }
        i += 2;
    }

    i = 0;
    while (trans_team[i] != NULL) {
        if (is_in_all_team_list(trans_team[i]) == 0) {
            all_team[count++] = trans_team[i];
            all_team[count] = NULL;
        }
        i += 3;
    }
}

static void generate_infocontrib(char *in_path, char *out_path, char *in_filename, char *out_filename, char *sed_filename)
{
    int found_start = 0;
    int found_end = 0;
    int found_trans = 0;
    int found_doc = 0;
    int i;
    int core_count = 0;
    int ex_count = 0;
    int trans_count = 0;
    int doc_count = 0;
    char *buffer;
    char *ptr;
    size_t line_size;
    FILE *infile, *outfile, *sedfile;

    sprintf(line_buffer, "%s%s", in_path, in_filename);
    infile = fopen(line_buffer, "rb");
    if (infile == NULL) {
        printf("cannot open %s for reading\n", line_buffer);
        return;
    }

    sprintf(line_buffer, "%s%s", in_path, sed_filename);
    sedfile = fopen(line_buffer, "rb");
    if (sedfile == NULL) {
        printf("cannot open %s for reading\n", line_buffer);
        fclose(infile);
        return;
    }

    sprintf(line_buffer, "%s%s", out_path, out_filename);
    outfile = fopen(line_buffer, "wb");
    if (outfile == NULL) {
        printf("cannot open %s for writing\n", line_buffer);
        fclose(infile);
        fclose(sedfile);
        return;
    }

    read_sed_file(sedfile);
    replace_tokens();

    fprintf(outfile, "/*\n");
    fprintf(outfile, " * infocontrib.h - Text of contributors to VICE, as used in info.c\n");
    fprintf(outfile, " *\n");
    fprintf(outfile, " * Autogenerated by geninfocontrib_h.sh, DO NOT EDIT !!!\n");
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
    fprintf(outfile, "#ifndef VICE_INFOCONTRIB_H\n");
    fprintf(outfile, "#define VICE_INFOCONTRIB_H\n\n");
#ifdef WINMIPS
    fprintf(outfile, "const char *info_contrib_text[] = {\n");
#else
    fprintf(outfile, "const char info_contrib_text[] =\n");
#endif

    while (found_start == 0) {
        line_size = get_line(infile);
        if (line_size >= strlen("@c ---vice-core-team---")) {
            if (!strncmp(line_buffer, "@c ---vice-core-team---", 23)) { 
                found_start = 1;
            }
        }
    }

    while (found_end == 0) {
        line_size = get_line(infile);
        if (line_buffer[0] != '@') {
            buffer = line_buffer;
            while (buffer[0] != '}') {
                buffer++;
            }
            buffer += 2;
            for (i = 0; buffer[i] != ' '; i++) {
            }
            buffer[i] = 0;
            core_team[core_count++] = vice_stralloc(buffer);
            buffer += (i + 1);
            core_team[core_count++] = vice_stralloc(buffer);
            core_team[core_count] = NULL;
        } else {
            found_end = 1;
        }
    }

    found_start = 0;
    found_end = 0;

    while (found_start == 0) {
        line_size = get_line(infile);
        if (line_size >= strlen("@c ---ex-team---")) {
            if (!strncmp(line_buffer, "@c ---ex-team---", 23)) { 
                found_start = 1;
            }
        }
    }

    while (found_end == 0) {
        line_size = get_line(infile);
        if (line_buffer[0] != '@') {
            buffer = line_buffer;
            while (buffer[0] != '}') {
                buffer++;
            }
            buffer += 2;
            for (i = 0; buffer[i] != ' '; i++) {
            }
            if (buffer[i - 1] == ',') {
                i++;
                while (buffer[i] != ' ') {
                    i++;
                }
            }
            buffer[i] = 0;
            ex_team[ex_count++] = vice_stralloc(buffer);
            buffer += (i + 1);
            ex_team[ex_count++] = vice_stralloc(buffer);
            ex_team[ex_count] = NULL;
        } else {
            found_end = 1;
        }
    }

    found_start = 0;
    found_end = 0;

    while (found_start == 0) {
        line_size = get_line(infile);
        if (line_size >= strlen("@chapter Acknowledgments")) {
            if (!strncmp(line_buffer, "@chapter Acknowledgments", 24)) { 
                found_start = 1;
            }
        }
    }

    while (found_end == 0) {
        line_size = get_line(infile);
        if (line_size == 0) {
#ifdef WINMIPS
            fprintf(outfile, "\"\\n\",\n");
#else
            fprintf(outfile, "\"\\n\"\n");
#endif
        } else {
            if (!strncmp(line_buffer, "@node Copyright, Contacts, Acknowledgments, Top", 47)) {
                found_end = 1;
            } else {
                if (found_trans == 1) {
                    if (!strcmp(line_buffer, "@c ---translation-team-end---")) {
                        found_trans = 0;
                    }
                }
                if (found_doc == 1) {
                    if (!strcmp(line_buffer, "@c ---documentation-team-end---")) {
                        found_doc = 0;
                    }
                }

                if (found_trans == 1) {
                    if (line_buffer[0] == '@') {
                        buffer = vice_stralloc(line_buffer);
                        ptr = buffer;
                        while (ptr[0] != '{') {
                            ptr++;
                        }
                        ptr++;
                        for (i = 0; ptr[i] != '}'; i++) {
                        }
                        ptr[i] = 0;
                        trans_team[trans_count++] = vice_stralloc(ptr);
                        free(buffer);
                    } else if (line_buffer[0] == 'P') {
                        buffer = vice_stralloc(line_buffer);
                        ptr = buffer;
                        while (ptr[0] != ' ') {
                            ptr++;
                        }
                        ptr++;
                        while (ptr[0] != ' ') {
                            ptr++;
                        }
                        ptr++;
                        for (i = 0; ptr[i] != ' '; i++) {
                        }
                        ptr[i] = 0;
                        trans_team[trans_count++] = vice_stralloc(ptr);
                        trans_team[trans_count] = NULL;
                        free(buffer);
                    } else if (line_buffer[0] == 'C') {
                        buffer = vice_stralloc(line_buffer);
                        ptr = buffer;
                        while (ptr[0] != '}') {
                            ptr++;
                        }
                        ptr += 2;
                        trans_team[trans_count++] = vice_stralloc(ptr);
                        free(buffer);
                    }
                }

                if (found_doc == 1) {
                    if (line_buffer[0] == '@') {
                        buffer = vice_stralloc(line_buffer);
                        ptr = buffer + 3;
                        for (i = 0; ptr[i] != '}'; i++) {
                        }
                        ptr[i] = 0;
                        doc_team[doc_count++] = vice_stralloc(ptr);
                        doc_team[doc_count] = NULL;
                        free(buffer);
                    }
                }

                if (found_trans == 0) {
                    if (!strcmp(line_buffer, "@c ---translation-team---")) {
                        found_trans = 1;
                    }
                }
                if (found_doc == 0) {
                    if (!strcmp(line_buffer, "@c ---documentation-team---")) {
                        found_doc = 1;
                    }
                }
                if (checklineignore() == 0) {
                    replacetags();
#ifdef WINMIPS
                    fprintf(outfile, "\"  %s\\n\",\n", line_buffer);
#else
                    fprintf(outfile, "\"  %s\\n\"\n", line_buffer);
#endif
                }
            }
        }
    }

#ifdef WINMIPS
    fprintf(outfile, "\"\\n\",\n};\n\n");
#else
    fprintf(outfile, "\"\\n\";\n\n");
#endif
    
    fprintf(outfile, "vice_team_t core_team[] = {\n");

    i = 0;
    while (core_team[i] != NULL) {
        sprintf(line_buffer, "@b{%s}", core_team[i + 1]);
        replacetags();
        fprintf(outfile, "    { \"%s\", \"%s\", \"%s\" },\n", core_team[i], core_team[i + 1], line_buffer);
        i += 2;
    }
    fprintf(outfile, "    { NULL, NULL, NULL }\n");
    fprintf(outfile, "};\n\n");

    fprintf(outfile, "vice_team_t ex_team[] = {\n");

    i = 0;
    while (ex_team[i] != NULL) {
        sprintf(line_buffer, "@b{%s}", ex_team[i + 1]);
        replacetags();
        fprintf(outfile, "    { \"%s\", \"%s\", \"%s\" },\n", ex_team[i], ex_team[i + 1], line_buffer);
        i += 2;
    }
    fprintf(outfile, "    { NULL, NULL, NULL }\n");
    fprintf(outfile, "};\n\n");

    fprintf(outfile, "char *doc_team[] = {\n");

    i = 0;
    while (doc_team[i] != NULL) {
        fprintf(outfile, "    \"%s\",\n", doc_team[i]);
        i++;
    }
    fprintf(outfile, "    NULL\n");
    fprintf(outfile, "};\n\n");

    fprintf(outfile, "vice_trans_t trans_team[] = {\n");

    i = 0;
    while (trans_team[i] != NULL) {
        sprintf(line_buffer, "@b{%s}", trans_team[i]);
        replacetags();
        fprintf(outfile, "    { \"%s\", \"%s\", \"%s\", \"%s\" },\n", trans_team[i + 1], trans_team[i], trans_team[i + 2], line_buffer);
        i += 3;
    }
    fprintf(outfile, "    { NULL, NULL, NULL, NULL }\n");
    fprintf(outfile, "};\n");

    fprintf(outfile, "#endif\n");

    fclose(infile);
    fclose(sedfile);
    fclose(outfile);
}

static void generate_authors(char *out_path, char *filename)
{
    FILE *outfile = NULL;
    int i = 0;

    sprintf(line_buffer, "%s%s", out_path, filename);
    outfile = fopen(line_buffer, "wb");

    if (outfile == NULL) {
        printf("cannot open %s for writing\n", line_buffer);
        return;
    }

    fprintf(outfile, "Core Team Members:\n\n");

    while (core_team[i] != NULL) {
        sprintf(line_buffer, "@b{%s}", core_team[i + 1]);
        replacetags();
        fprintf(outfile, "%s\n", line_buffer);
        i += 2;
    }

    fprintf(outfile, "\n\nInactive/Ex Team Members:\n\n");

    i = 0;
    while (ex_team[i] != NULL) {
        sprintf(line_buffer, "@b{%s}", ex_team[i + 1]);
        replacetags();
        fprintf(outfile, "%s\n", line_buffer);
        i += 2;
    }

    fprintf(outfile, "\n\nTranslation Team Members:\n\n");

    i = 0;
    while (trans_team[i] != NULL) {
        sprintf(line_buffer, "@b{%s}", trans_team[i]);
        replacetags();
        fprintf(outfile, "%s\n", line_buffer);
        i += 3;
    }
    fclose(outfile);
}

static void generate_osx_credits_html(char *out_path, char *filename)
{
    FILE *outfile = NULL;
    int i = 0;

    sprintf(line_buffer, "%s%s", out_path, filename);
    outfile = fopen(line_buffer, "wb");

    if (outfile == NULL) {
        printf("cannot open %s for writing\n", line_buffer);
        return;
    }
    fprintf(outfile, "<html>\n");
    fprintf(outfile, "<head><title>VICE Credits</title></head>\n");
    fprintf(outfile, "<body>\n");
    fprintf(outfile, "<div align=\"center\">VICE Core Team Members:</div>\n");
    fprintf(outfile, "<ul>\n");

    while (core_team[i] != NULL) {
        sprintf(line_buffer, "@b{%s}", core_team[i + 1]);
        replacetags();
        fprintf(outfile, "<li>%s</li>\n", core_team[i + 1]);
        i += 2;
    }

    fprintf(outfile, "</ul>\n");
    fprintf(outfile, "<div align=\"center\">Ex/Inactive Team Members:</div>\n");
    fprintf(outfile, "<ul>\n");

    i = 0;
    while (ex_team[i] != NULL) {
        fprintf(outfile, "<li>%s</li>\n", ex_team[i + 1]);
        i += 2;
    }

    fprintf(outfile, "</ul>\n");
    fprintf(outfile, "<div align=\"center\">The VICE Translation Team:</div>\n");
    fprintf(outfile, "<ul>\n");

    i = 0;
    while (trans_team[i] != NULL) {
        fprintf(outfile, "<li>%s</li>\n", trans_team[i]);
        i += 3;
    }

    fprintf(outfile, "</ul>\n");
    fprintf(outfile, "</body>\n");
    fprintf(outfile, "</html>\n");

    fclose(outfile);
}

static void generate_readme(char *in_path, char *out_path, char *filename)
{
    FILE *infile = NULL;
    FILE *outfile = NULL;
    int i = 0;
    int found_start = 0;
    int found_end = 0;
    int found_eof = 0;
    int line_size;
    char *tmpname;

    sprintf(line_buffer, "%s%s", in_path, filename);
    infile = fopen(line_buffer, "rb");
    if (infile == NULL) {
        printf("cannot open %s for reading\n", line_buffer);
        return;
    }

    sprintf(line_buffer, "%s%s.tmp", out_path, filename);
    outfile = fopen(line_buffer, "wb");
    if (outfile == NULL) {
        printf("cannot open %s for writing\n", line_buffer);
        fclose(infile);
        return;
    }

    while (found_start == 0) {
        line_size = get_line(infile);
        if (!strcmp(line_buffer, " VICE, the Versatile Commodore Emulator")) { 
            found_start = 1;
        }
        if (line_size == 0) {
            fprintf(outfile, "\n");
        } else {
            fprintf(outfile, "%s\n", line_buffer);
        }
    }

    fprintf(outfile, "\n    Core Team Members:\n");

    while (core_team[i] != NULL) {
        fprintf(outfile, "    %s %s\n", core_team[i], core_team[i + 1]);
        i += 2;
    }

    fprintf(outfile, "\n    Inactive/Ex Team Members:\n");

    i = 0;
    while (ex_team[i] != NULL) {
        fprintf(outfile, "    %s %s\n", ex_team[i], ex_team[i + 1]);
        i += 2;
    }

    fprintf(outfile, "\n    Translation Team Members:\n");

    i = 0;
    while (trans_team[i] != NULL) {
        fprintf(outfile, "    %s %s\n", trans_team[i + 1], trans_team[i]);
        i += 3;
    }

    fprintf(outfile, "\n");
    fprintf(outfile, "  This program is free software; you can redistribute it and/or\n");

    while (found_end == 0) {
        line_size = get_line(infile);
        if (!strcmp(line_buffer, "  This program is free software; you can redistribute it and/or")) { 
            found_end = 1;
        }
    }

    while (found_eof == 0) {
        line_size = get_line(infile);
        if (line_size == -1) {
            found_eof = 1;
        } else {
            if (line_size == 0) {
                fprintf(outfile, "\n");
            } else {
                fprintf(outfile, "%s\n", line_buffer);
            }
        }
    }

    fclose(outfile);
    fclose(infile);

    sprintf(line_buffer, "%s%s", out_path, filename);
    tmpname = vice_stralloc(line_buffer);
    sprintf(line_buffer, "%s%s.tmp", out_path, filename);
    unlink(tmpname);
    rename(line_buffer, tmpname);
    free(tmpname);
}

static void generate_index_html(char *in_path, char *out_path, char *filename)
{
    FILE *infile = NULL;
    FILE *outfile = NULL;
    int i = 0;
    int found_start = 0;
    int found_end = 0;
    int found_eof = 0;
    int line_size;
    char *tmpname;

    sprintf(line_buffer, "%s%s", in_path, filename);
    infile = fopen(line_buffer, "rb");
    if (infile == NULL) {
        printf("cannot open %s for reading\n", line_buffer);
        return;
    }

    sprintf(line_buffer, "%s%s.tmp", out_path, filename);
    outfile = fopen(line_buffer, "wb");
    if (outfile == NULL) {
        printf("cannot open %s for writing\n", line_buffer);
        fclose(infile);
        return;
    }

    fill_all_team_list();

    while (found_start == 0) {
        line_size = get_line(infile);
        if (!strcmp(line_buffer, "<!--teamstart-->")) { 
            found_start = 1;
        }
        if (line_size == 0) {
            fprintf(outfile, "\n");
        } else {
            fprintf(outfile, "%s\n", line_buffer);
        }
    }

    fprintf(outfile, "<p>\n");
    fprintf(outfile, "Current VICE team members:\n");

    fprintf(outfile, "%s", core_team[i + 1]);
    i = 2;
    while (core_team[i] != NULL) {
        fprintf(outfile, ",\n");
        fprintf(outfile, "%s", core_team[i + 1]);
        i += 2;
    }
    fprintf(outfile, ".\n");
    fprintf(outfile, "</p>\n\n");
    fprintf(outfile, "<p>Of course our warm thanks go to everyone who has helped us in developing\n");
    fprintf(outfile, "VICE during these past few years. For a more detailed list look in the\n");
    fprintf(outfile, "<a href=\"vice_16.html\">documentation</a>.\n\n\n");
    fprintf(outfile, "<hr>\n\n");
    fprintf(outfile, "<h1><a NAME=\"copyright\"></a>Copyright</h1>\n\n");
    fprintf(outfile, "<p>\n");
    fprintf(outfile, "The VICE is copyrighted to\n");

    fprintf(outfile, "%s", all_team[0]);
    for (i = 1; all_team[i]; i++) {
        fprintf(outfile, ",\n%s", all_team[i]);
    }
    fprintf(outfile, ".\n");
    fprintf(outfile, "<!--teamend-->\n");

    while (found_end == 0) {
        line_size = get_line(infile);
        if (!strcmp(line_buffer, "<!--teamend-->")) { 
            found_end = 1;
        }
    }

    while (found_eof == 0) {
        line_size = get_line(infile);
        if (line_size == -1) {
            found_eof = 1;
        } else {
            if (line_size == 0) {
                fprintf(outfile, "\n");
            } else {
                fprintf(outfile, "%s\n", line_buffer);
            }
        }
    }

    fclose(outfile);
    fclose(infile);

    sprintf(line_buffer, "%s%s", out_path, filename);
    tmpname = vice_stralloc(line_buffer);
    sprintf(line_buffer, "%s%s.tmp", out_path, filename);
    unlink(tmpname);
    rename(line_buffer, tmpname);
    free(tmpname);
}

static void generate_vice_1(char *in_path, char *out_path, char *filename)
{
    FILE *infile = NULL;
    FILE *outfile = NULL;
    int i = 0;
    char *tmpname;
    int found_start = 0;
    int line_size;

    sprintf(line_buffer, "%s%s", in_path, filename);
    infile = fopen(line_buffer, "rb");
    if (infile == NULL) {
        printf("cannot open %s for reading\n", line_buffer);
        return;
    }

    sprintf(line_buffer, "%s%s.tmp", out_path, filename);
    outfile = fopen(line_buffer, "wb");
    if (outfile == NULL) {
        printf("cannot open %s for writing\n", line_buffer);
        fclose(infile);
        return;
    }

    fill_all_team_list();

    while (found_start == 0) {
        line_size = get_line(infile);
        if (!strcmp(line_buffer, ".SH AUTHORS")) { 
            found_start = 1;
        }
        if (line_size == 0) {
            fprintf(outfile, "\n");
        } else {
            fprintf(outfile, "%s\n", line_buffer);
        }
    }

    for (i = 0; all_team[i]; i++) {
        sprintf(line_buffer, "@b{%s}", all_team[i]);
        replacetags();
        fprintf(outfile, "%s\n.br\n", line_buffer);
    }
    fprintf(outfile, "with several contributions from other people around the world; see the\n");
    fprintf(outfile, "HTML documentation for more information.\n\n\n");

    fclose(outfile);
    fclose(infile);

    sprintf(line_buffer, "%s%s", out_path, filename);
    tmpname = vice_stralloc(line_buffer);
    sprintf(line_buffer, "%s%s.tmp", out_path, filename);
    unlink(tmpname);
    rename(line_buffer, tmpname);
    free(tmpname);
}

int main(int argc, char *argv[])
{
    int i;
    if (argc < 11) {
        printf("too few arguments\n");
        exit(1);
    }

    /* argv[1] = top src dir */
    /* argv[2] = top build dir */
    /* argv[3] = vice.texi file for reading */
    /* argv[4] = infocontrib.h file for writing */
    /* argv[5] = infocontrib.c file for reading */
    /* argv[6] = AUTHORS file for writing */
    /* argv[7] = Credits.html file for writing */
    /* argv[8] = README file for reading and writing */
    /* argv[9] = index.html file for reading and writing */
    /* argv[10] = dialogs.rc file for reading and writing */
    /* argv[11] = vice.1 file for writing */

    generate_infocontrib(argv[1], argv[2], argv[3], argv[4], argv[5]);

    generate_authors(argv[2], argv[6]);

    generate_osx_credits_html(argv[2], argv[7]);

    generate_readme(argv[1], argv[2], argv[8]);

    generate_index_html(argv[1], argv[2], argv[9]);

    generate_vice_1(argv[1], argv[2], argv[10]);

    for (i = 0; core_team[i] != NULL; i++) {
        free(core_team[i++]);
    }

    for (i = 0; ex_team[i] != NULL; i++) {
        free(ex_team[i++]);
    }

    for (i = 0; trans_team[i] != NULL; i++) {
        free(trans_team[i++]);
    }

    for (i = 0; doc_team[i] != NULL; i++) {
        free(doc_team[i++]);
    }

    return 0;
}
