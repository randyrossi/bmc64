/*
 * fixdeps.c - Dependency fixer helper program for AmigaOS4 native compile.
 *
 * Written by
 *  Peter Gordon <pete@petergordon.org.uk>
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
#include <string.h>

#include <exec/types.h>

#include <proto/exec.h>
#include <proto/dos.h>

static TEXT *buf = NULL;;
static uint32 buflen = 0;

static BOOL iseol(TEXT c)
{
    if ((c == 10) || (c == 13)) {
        return TRUE;
    }

    return FALSE;
}

static BOOL isws(TEXT c)
{
    if ((c == 32) || (c == 9)) {
        return TRUE;
    }

    return FALSE;
}

static BOOL isech(TEXT c)
{
    if ((c == 10) || (c == 13)) {
        return TRUE;
    }

    if ((c == 32) || (c == 9)) {
        return TRUE;
    }

    return FALSE;
}

static BOOL init(void)
{
    return TRUE;
}

static void shut(void)
{
    if (buf) {
        IExec->FreeVec(buf);
    }
}

static void reallocbuf(uint32 size)
{
    if (!buf) {
        buf = IExec->AllocVec(size, MEMF_ANY);
        if (!buf) {
            return;
        }
        buflen = size;
        return;
    }
  
    if (size <= buflen) {
        return;
    }

    IExec->FreeVec(buf);
    buf = IExec->AllocVec(size, MEMF_ANY);
    if (!buf) {
        return;
    }
    buflen = size;
}

static void scan(TEXT *dir, TEXT *dirsofar)
{
    APTR dcontext = NULL;
    struct ExamineData *data = NULL;
    uint32 i, j, ndsl, changes;
    TEXT *ndirsofar, *obuf;
    BPTR h;
    int32 s, d, css, dcss, ls, dls;
    BOOL indeps;

    if (dirsofar == NULL) {
        dirsofar = "";
    }

    ndsl = strlen(dirsofar) + strlen(dir) + 4;
    ndirsofar = IExec->AllocVec(ndsl, MEMF_ANY);
    if (ndirsofar) {
        strcpy(ndirsofar, dirsofar);
        IDOS->AddPart(ndirsofar, dir, ndsl);
    }
  
    dcontext = IDOS->ObtainDirContextTags(EX_StringName, dir,
                                          EX_DataFields, EXF_NAME,
                                          EX_DoCurrentDir, TRUE,
                                          TAG_DONE);
    if (!dcontext) {
        if (ndirsofar) {
            IExec->FreeVec(ndirsofar);
        }
        return;
    }
  
    while ((data = IDOS->ExamineDir(dcontext))) {
        if (EXD_IS_FILE(data)) {
            if ((i = strlen(data->Name)) > 3) {
                if (strcasecmp(&data->Name[i - 3], ".Po") == 0) {
                    if ((ndirsofar) && (ndirsofar[0])) {
                        printf("%s/%s: ", ndirsofar, data->Name);
                    } else {
                        printf("%s: ", data->Name);
                    }

                    reallocbuf( data->FileSize * 3 );

                    if (!buf) {
                        printf("Unable to allocate %lld bytes.\n", data->FileSize * 3);
                        continue;
                    }

                    h = IDOS->Open(data->Name, MODE_OLDFILE);
                    if (!h) {
                        printf("Open() failed.\n");
                        continue;
                    }

                    IDOS->Read(h, buf, data->FileSize);
                    IDOS->Close(h);

                    changes = 0;
                    obuf = &buf[data->FileSize];
          
                    ls = 0;
                    dls = 0;
                    s = 0;
                    d = 0;
                    css = -1;
                    dcss = -1;
                    indeps = FALSE;
                    while (s < data->FileSize) {
                        if (!indeps) {
                            if (iseol(buf[s])) {
                                ls = s + 1;
                                dls = d + 1;
                            }

                            if (buf[s] == ':') {
                                if (isech(buf[s + 1])) {
                                    indeps = TRUE;
                                    css = s + 1;
                                    dcss = d + 1;
                                } else {
                                    d = dls;
                                    obuf[d++] = '/';
                                    for (j = ls; j < s; j++) {
                                        obuf[d++] = buf[j];
                                    }
                                    obuf[d++] = '/';
                                    s++;
                                    changes++;
                                }
                            }
                        } else {
                            if ((buf[s] == '\\') && (iseol(buf[s + 1]))) {
                                obuf[d++] = buf[s++];
                                if (buf[s] == 13) {
                                    obuf[d++] = buf[s++];
                                }
                                if (buf[s] == 10) {
                                    obuf[d++] = buf[s++];
                                }
                                continue;
                            }
              
                            if (isws(buf[s])) {
                                css = s + 1;
                                dcss = d + 1;
                            }
              
                            if (iseol(buf[s])) {
                                indeps = FALSE;
                                obuf[d++] = buf[s++];
                                ls = s;
                                dls = d;
                                continue;
                            }
              
                            if (buf[s] == ':') {
                                if ((dcss == -1) || (css == s)) {
                                    obuf[d++] = buf[s++];
                                    continue;
                                }
              
                                d = dcss;
                                obuf[d++] = '/';
                                for (j = css; j < s; j++) {
                                    obuf[d++] = buf[j];
                                }
                                obuf[d++] = '/';
                                s++;
                                changes++;
                                continue;
                            }
                        }
            
                        if (s < data->FileSize) {
                            obuf[d++] = buf[s++];
                        }
                    }
          
                    if (changes) {
                        h = IDOS->Open(data->Name, MODE_NEWFILE);
                        if (h) {
                            IDOS->Write(h, obuf, d);
                            IDOS->Close(h);
                        }
                    }
          
                    printf( "%ld change(s).\n", changes );
                }
            }
            continue;
        }
    
        if (EXD_IS_DIRECTORY(data)) {
            scan(data->Name, ndirsofar);
            continue;
        }
    }
  
    IDOS->ReleaseDirContext(dcontext);
    IExec->FreeVec(ndirsofar);
}

int main(void)
{
    if (init()) {
        scan("", "");
    }
    shut();
    return 0;
}
