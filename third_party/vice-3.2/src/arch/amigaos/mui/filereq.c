/*
 * filereq.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include "vice.h"

#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif

#include "mui.h"

#include "lib.h"
#include "imagecontents.h"
#include "diskimage.h"
#include "util.h"
#include "ui.h"
#include "vdrive-internal.h"
#include "cbmimage.h"
#include "filereq.h"
#include "intl.h"
#include "translate.h"

#ifndef AMIGA_OS4
#include <sys/cdefs.h>
#endif

#ifdef AMIGA_MORPHOS
#include <clib/alib_protos.h>
#endif

struct ObjApp {
    APTR App;
    APTR FILEREQ;
    APTR PA_FILEREQ;
    APTR STR_PA_FILEREQ;
    APTR LV_FILELIST;
    APTR LV_VOLUMELIST;
    APTR BT_ATTACH;
    APTR BT_PARENT;
    APTR BT_CANCEL;
    APTR LV_CONTENTS;
    APTR CH_READONLY;
    APTR STR_IMAGENAME;
    APTR STR_IMAGEID;
    APTR CY_IMAGETYPE;
    APTR BT_CREATEIMAGE;
    APTR BT_CREATETAPIMAGE;
};

static const char *image_type_name[] = {
    "d64",
    "d67",
    "d71",
    "d80",
    "d81",
    "d82",
    "d1m",
    "d2m",
    "d4m",
    "g64",
    "p64",
    "x64",
    NULL
};

static const int image_type[] = {
    DISK_IMAGE_TYPE_D64,
    DISK_IMAGE_TYPE_D67,
    DISK_IMAGE_TYPE_D71,
    DISK_IMAGE_TYPE_D80,
    DISK_IMAGE_TYPE_D81,
    DISK_IMAGE_TYPE_D82,
    DISK_IMAGE_TYPE_D1M,
    DISK_IMAGE_TYPE_D2M,
    DISK_IMAGE_TYPE_D4M,
    DISK_IMAGE_TYPE_G64,
    DISK_IMAGE_TYPE_P64,
    DISK_IMAGE_TYPE_X64,
    -1
};

static struct ObjApp * CreateApp(const char *title, int template, char *resource_readonly, char *initialdir)
{
    struct ObjApp *Object_ok;

    APTR GROUP_ROOT, GR_FILEREQ, LA_FILEREQ, GR_FILEBROWSE, GR_BUTTONS, GR_MISC_1;
    APTR GR_CONTENTS, GR_MISC_2, GR_READONLY, LA_READONLY, GR_NEWIMAGE, GR_MISC_3;
    APTR GR_NEWTAPIMAGE;
    char *new_image_text;
    char *new_tap_image_text;
    char *image_contents_text;

    if (!(Object_ok = lib_AllocVec(sizeof(struct ObjApp), MEMF_PUBLIC | MEMF_CLEAR))) {
        return NULL;
    }

    LA_FILEREQ = Label(translate_text(IDMS_FILE));

    Object_ok->STR_PA_FILEREQ = String("", 1024);

    Object_ok->PA_FILEREQ = PopButton(MUII_PopFile);

    Object_ok->PA_FILEREQ = PopaslObject,
                              MUIA_HelpNode, "PA_FILEREQ",
                              MUIA_Popasl_Type, 0,
                              MUIA_Popstring_String, Object_ok->STR_PA_FILEREQ,
                              MUIA_Popstring_Button, Object_ok->PA_FILEREQ,
                            End;

    GR_FILEREQ = GroupObject,
                   MUIA_HelpNode, "GR_FILEREQ",
                   MUIA_Group_Horiz, TRUE,
		       Child, LA_FILEREQ,
                   Child, Object_ok->PA_FILEREQ,
                 End;

    Object_ok->LV_FILELIST = DirlistObject,
                               MUIA_Background, MUII_ListBack,
                               MUIA_Frame, MUIV_Frame_InputList,
                               MUIA_Dirlist_Directory, initialdir,
                             End;

    Object_ok->LV_FILELIST = ListviewObject,
                               MUIA_HelpNode, "LV_FILELIST",
                               MUIA_Listview_List, Object_ok->LV_FILELIST,
                             End;

    Object_ok->LV_VOLUMELIST = VolumelistObject,
                                 MUIA_Frame, MUIV_Frame_InputList,
                               End;

    Object_ok->LV_VOLUMELIST = ListviewObject,
                                 MUIA_HelpNode, "LV_VOLUMELIST",
                                 MUIA_Weight, 30,
                                 MUIA_Listview_MultiSelect, MUIV_Listview_MultiSelect_None,
                                 MUIA_Listview_List, Object_ok->LV_VOLUMELIST,
                               End;

    GR_FILEBROWSE = GroupObject,
                      MUIA_HelpNode, "GR_FILEBROWSE",
                      MUIA_Group_Horiz, TRUE,
                      Child, Object_ok->LV_FILELIST,
                      Child, Object_ok->LV_VOLUMELIST,
                    End;

    Object_ok->BT_ATTACH = SimpleButton(translate_text(IDS_ATTACH));

    Object_ok->BT_PARENT = SimpleButton(translate_text(IDS_PARENT));

    Object_ok->BT_CANCEL = SimpleButton(translate_text(IDS_CANCEL));

    GR_BUTTONS = GroupObject,
                   MUIA_HelpNode, "GR_BUTTONS",
                   MUIA_Group_Horiz, TRUE,
                   Child, Object_ok->BT_ATTACH,
                   Child, Object_ok->BT_PARENT,
                   Child, Object_ok->BT_CANCEL,
                 End;

    Object_ok->LV_CONTENTS = ListObject,
                               MUIA_Frame, MUIV_Frame_InputList,
                               MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
                               MUIA_List_DestructHook, MUIV_List_DestructHook_String,
                             End;

    Object_ok->LV_CONTENTS = ListviewObject,
                               MUIA_HelpNode, "LV_CONTENTS",
                               MUIA_Listview_MultiSelect, MUIV_Listview_MultiSelect_None,
                               MUIA_Listview_List, Object_ok->LV_CONTENTS,
                             End;

    image_contents_text = translate_text(IDS_IMAGE_CONTENTS);

    GR_CONTENTS = GroupObject,
                    MUIA_HelpNode, "GR_CONTENTS",
                    MUIA_Frame, MUIV_Frame_Group,
                    MUIA_FrameTitle, image_contents_text,
                    Child, Object_ok->LV_CONTENTS,
                  End;

    Object_ok->CH_READONLY = CheckMark(FALSE);

    LA_READONLY = Label(translate_text(IDS_ATTACH_READ_ONLY));

    GR_READONLY = GroupObject,
                    MUIA_HelpNode, "GR_READONLY",
                    MUIA_Group_Horiz, TRUE,
                    Child, Object_ok->CH_READONLY,
                    Child, LA_READONLY,
                  End;

    Object_ok->STR_IMAGENAME = StringObject,
                                 MUIA_Frame, MUIV_Frame_String,
                                 MUIA_FrameTitle, translate_text(IDS_NAME),
                                 MUIA_HelpNode, "STR_IMAGENAME",
                                 MUIA_String_MaxLen, 17,
                               End;

    Object_ok->STR_IMAGEID = StringObject,
                               MUIA_Frame, MUIV_Frame_String,
                               MUIA_FrameTitle, "ID",
                               MUIA_HelpNode, "STR_IMAGEID",
                               MUIA_String_MaxLen, 5,
                             End;

    Object_ok->CY_IMAGETYPE = CycleObject,
                                MUIA_HelpNode, "CY_IMAGETYPE",
                                MUIA_Cycle_Entries, image_type_name,
                              End;

    GR_MISC_3 = GroupObject,
                  MUIA_HelpNode, "GR_MISC_3",
                  MUIA_Group_Horiz, TRUE,
                  Child, Object_ok->STR_IMAGEID,
                  Child, Object_ok->CY_IMAGETYPE,
                End;

    Object_ok->BT_CREATEIMAGE = SimpleButton(translate_text(IDS_CREATE_IMAGE));

    new_image_text = translate_text(IDS_NEW_IMAGE);

    GR_NEWIMAGE = GroupObject,
                    MUIA_HelpNode, "GR_NEWIMAGE",
                    MUIA_Frame, MUIV_Frame_Group,
                    MUIA_FrameTitle, new_image_text,
                    Child, Object_ok->STR_IMAGENAME,
                    Child, GR_MISC_3,
                    Child, Object_ok->BT_CREATEIMAGE,
                  End;

    Object_ok->BT_CREATETAPIMAGE = SimpleButton(translate_text(IDS_CREATE_IMAGE));

    new_tap_image_text = translate_text(IDS_NEW_TAP_IMAGE);

    GR_NEWTAPIMAGE = GroupObject,
                       MUIA_HelpNode, "GR_NEWTAPIMAGE",
                       MUIA_Frame, MUIV_Frame_Group,
                       MUIA_FrameTitle, new_tap_image_text,
                       Child, Object_ok->BT_CREATETAPIMAGE,
                     End;

    GR_MISC_2 = GroupObject,
                  MUIA_HelpNode, "GR_MISC_2",
                  Child, GR_READONLY,
                  Child, GR_NEWIMAGE,
                  Child, GR_NEWTAPIMAGE,
                End;

    GR_MISC_1 = GroupObject,
                  MUIA_HelpNode, "GR_MISC_1",
                  MUIA_Group_Horiz, TRUE,
                  Child, GR_CONTENTS,
                  Child, GR_MISC_2,
                End;

    GROUP_ROOT = GroupObject,
                   Child, GR_FILEREQ,
                   Child, GR_FILEBROWSE,
                   Child, GR_BUTTONS,
                   Child, GR_MISC_1,
                 End;

    Object_ok->FILEREQ = WindowObject,
                           MUIA_Window_Title, title,
                           MUIA_Window_ID, MAKE_ID('F', 'R', 'E', 'Q'),
                           MUIA_Window_Screen, canvaslist->os->screen,
                           WindowContents, GROUP_ROOT,
                         End;

    Object_ok->App = ApplicationObject,
                       MUIA_Application_Author, "NONE",
                       MUIA_Application_Base, "NONE",
                       MUIA_Application_Title, "NONE",
                       MUIA_Application_Version, "_VER: NONE XX.XX (XX.XX.XX)",
                       MUIA_Application_Copyright, "NOBODY",
                       MUIA_Application_Description, "NONE",
                       SubWindow, Object_ok->FILEREQ,
                     End;

    if (!Object_ok->App) {
        lib_FreeVec(Object_ok);
        return NULL;
    }

    DoMethod(Object_ok->FILEREQ, MUIM_Window_SetCycleChain,
             Object_ok->PA_FILEREQ,
             Object_ok->LV_FILELIST,
             Object_ok->LV_VOLUMELIST,
             Object_ok->BT_ATTACH,
             Object_ok->BT_PARENT,
             Object_ok->BT_CANCEL,
             Object_ok->LV_CONTENTS,
             Object_ok->CH_READONLY,
             Object_ok->STR_IMAGENAME,
             Object_ok->STR_IMAGEID,
             Object_ok->CY_IMAGETYPE,
             Object_ok->BT_CREATEIMAGE,
             Object_ok->BT_CREATETAPIMAGE,
             0);

    if (resource_readonly == NULL) {
        set(GR_READONLY, MUIA_ShowMe, FALSE);
    }

    if (template == IDD_OPENTAPE_TEMPLATE) {
        set(GR_READONLY, MUIA_ShowMe, FALSE);
        set(GR_NEWIMAGE, MUIA_ShowMe, FALSE);
    } else {
        set(GR_NEWTAPIMAGE, MUIA_ShowMe, FALSE);
    }

    set(Object_ok->FILEREQ, MUIA_Window_Open, TRUE);

    return(Object_ok);
}

static void DisposeApp(struct ObjApp *Object_ok)
{
    MUI_DisposeObject(Object_ok->App);
    lib_FreeVec(Object_ok);
}

static struct ObjApp *app = NULL;

static ULONG NewVolume(struct Hook *hook, Object *obj, APTR arg)
{
    char *buf;

    DoMethod(app->LV_VOLUMELIST, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &buf);
    set(app->LV_FILELIST, MUIA_Dirlist_Directory, buf);
    set(app->STR_PA_FILEREQ, MUIA_String_Contents, buf);

    return 0;
}

static void create_content_list(image_contents_t *contents, Object *list)
{
    char *start;
    image_contents_file_list_t *p = contents->file_list;

    DoMethod(list, MUIM_List_Clear);

    start = image_contents_to_string(contents, 0);
    DoMethod(list, MUIM_List_InsertSingle, start, MUIV_List_Insert_Bottom);
    lib_free(start);

    if (p == NULL) {
        DoMethod(list, MUIM_List_InsertSingle, "(EMPTY IMAGE.)", MUIV_List_Insert_Bottom);
    }
    else do {
        start = image_contents_file_to_string(p, 0);
        DoMethod(list, MUIM_List_InsertSingle, start, MUIV_List_Insert_Bottom);
        lib_free(start);
    } while ( (p = p->next) != NULL);

    if (contents->blocks_free >= 0) {
        start = lib_msprintf("%d BLOCKS FREE.", contents->blocks_free);
        DoMethod(list, MUIM_List_InsertSingle, start, MUIV_List_Insert_Bottom);
        lib_free(start);
    }
}

static image_contents_t *(*read_content_func)(const char *) = NULL;

enum {
    RET_NONE = 0,
    RET_AUTOSTART_ATTACH,
    RET_ATTACH
};

static ULONG NewFile(struct Hook *hook, Object *obj, APTR arg)
{
    char *buf;
    image_contents_t *contents;

    get(app->LV_FILELIST, MUIA_Dirlist_Path, (APTR)&buf);
    if (buf != NULL) {
        struct FileInfoBlock *fib;

        DoMethod(app->LV_FILELIST, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &fib);
        if (fib->fib_DirEntryType <= 0) {
            set(app->STR_PA_FILEREQ, MUIA_String_Contents, buf);

            if (read_content_func != NULL) {
                contents = read_content_func(buf);
                if (contents != NULL) {
                    create_content_list(contents, app->LV_CONTENTS);
                    image_contents_destroy(contents);
                }
            }
        }
    }

    return 0;
}

static ULONG NewDir(struct Hook *hook, Object *obj, APTR arg)
{
    char *buf;

    get(app->LV_FILELIST, MUIA_Dirlist_Path, (APTR)&buf);
    if (buf != NULL) {
        struct FileInfoBlock *fib;

        DoMethod(app->LV_FILELIST, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &fib);
        if (fib->fib_DirEntryType > 0) {
            char newdir[1024];

            strcpy(newdir, buf);
            AddPart(newdir, "", 1024);

            set(app->LV_FILELIST, MUIA_Dirlist_Directory, newdir);
            set(app->STR_PA_FILEREQ, MUIA_String_Contents, newdir);
        } else {
            DoMethod(app->App, MUIM_Application_ReturnID, RET_ATTACH);
        }
    }

    return 0;
}

static ULONG NewParentDir(struct Hook *hook, Object *obj, APTR arg)
{
    char *buf;

    get(app->LV_FILELIST, MUIA_Dirlist_Directory, (APTR)&buf);
    if (buf != NULL) {
        char newdir[1024], *ptr = PathPart(buf);

        memcpy(newdir, buf, (int)(ptr - buf));
        newdir[(int)(ptr - buf)] = '\0';

        set(app->LV_FILELIST, MUIA_Dirlist_Directory, newdir);
        set(app->STR_PA_FILEREQ, MUIA_String_Contents, newdir);
    }

    return 0;
}

static ULONG NewPopFile(struct Hook *hook, Object *obj, APTR arg)
{
    char *buf;
    image_contents_t *contents;

    get(app->STR_PA_FILEREQ, MUIA_String_Contents, (APTR)&buf);
    if (buf != NULL) {
        char newdir[1024], *ptr = PathPart(buf);

        memcpy(newdir, buf, (int)(ptr - buf));
        newdir[(int)(ptr - buf)] = '\0';

        set(app->LV_FILELIST, MUIA_Dirlist_Directory, newdir);

        if (read_content_func != NULL) {
            contents = read_content_func(buf);
            if (contents != NULL) {
                create_content_list(contents, app->LV_CONTENTS);
                image_contents_destroy(contents);
            }
        }
    }

    return 0;
}

static ULONG NewCreateImage(struct Hook *hook, Object *obj, APTR arg)
{
    char name[1024], *ext, *format_name;
    char *filename = NULL, *disk_name = NULL, *disk_id = NULL;
    int imagetype = 0;
    image_contents_t *contents;

    get(app->STR_PA_FILEREQ, MUIA_String_Contents, (APTR)&filename);
    get(app->STR_IMAGENAME, MUIA_String_Contents, (APTR)&disk_name);
    get(app->STR_IMAGEID, MUIA_String_Contents, (APTR)&disk_id);
    get(app->CY_IMAGETYPE, MUIA_Cycle_Active, (APTR)&imagetype);

    /*  Find last dot in name */
    ext = strrchr(filename, '.');

    strcpy(name, filename);
    if ((ext == NULL) || (strcasecmp(ext + 1, image_type_name[imagetype]) != 0)) {
        strcat(name, ".");
        strcat(name, image_type_name[imagetype]);
    }

    if (util_file_exists(name)) {
        LONG result = MUI_RequestA(app->App, app->FILEREQ, 0, translate_text(IDS_VICE_QUESTION),
                                   translate_text(IDS_YES_NO), translate_text(IDS_OVERWRITE_EXISTING_IMAGE), NULL);
        if (result != 1) {
            return 0;
        }
    }

    format_name = lib_msprintf("%s,%s", disk_name, disk_id);
    if (vdrive_internal_create_format_disk_image(name, format_name, image_type[imagetype]) < 0) {
        ui_error(translate_text(IDMES_CANNOT_CREATE_IMAGE));
    } else {
        char newdir[1024], *ptr = PathPart(name);

        memcpy(newdir, name, (int)(ptr - name));
        newdir[(int)(ptr - name)] = '\0';

        set(app->LV_FILELIST, MUIA_Dirlist_Directory, newdir);
        DoMethod(app->LV_FILELIST, MUIM_Dirlist_ReRead);
        set(app->STR_PA_FILEREQ, MUIA_String_Contents, name);

        if (read_content_func != NULL) {
            contents = read_content_func(name);
            create_content_list(contents, app->LV_CONTENTS);
            image_contents_destroy(contents);
        }
    }
    lib_free(format_name);

    return 0;
}

static ULONG NewCreateTAPImage(struct Hook *hook, Object *obj, APTR arg)
{
    char name[1024], *ext, *filename = NULL;

    get(app->STR_PA_FILEREQ, MUIA_String_Contents, (APTR)&filename);

    /*  Find last dot in name */
    ext = strrchr(filename, '.');

    strcpy(name, filename);
    if ((ext == NULL) || (strcasecmp(ext + 1, "tap") != 0)) {
        strcat(name, ".");
        strcat(name, "tap");
    }

    if (util_file_exists(name)) {
        LONG result = MUI_RequestA(app->App, app->FILEREQ, 0, translate_text(IDS_VICE_QUESTION),
                                   translate_text(IDS_YES_NO), translate_text(IDS_OVERWRITE_EXISTING_IMAGE), NULL);
        if (result != 1) {
            return 0;
        }
    }

    if (cbmimage_create_image(name, DISK_IMAGE_TYPE_TAP)) {
        ui_error(translate_text(IDMES_CANNOT_CREATE_IMAGE));
    } else {
        char newdir[1024], *ptr = PathPart(name);

        memcpy(newdir, name, (int)(ptr - name));
        newdir[(int)(ptr - name)] = '\0';

        set(app->LV_FILELIST, MUIA_Dirlist_Directory, newdir);
        DoMethod(app->LV_FILELIST, MUIM_Dirlist_ReRead);
        set(app->STR_PA_FILEREQ, MUIA_String_Contents, name);
    }

    return 0;
}

static char *request_file(const char *title, char *initialdir, char *initialfile)
{
    struct FileRequester *FileReq = NULL;
    static char file_name[1024];

    FileReq = AllocAslRequestTags(ASL_FileRequest, TAG_END);

    file_name[0] = '\0';
    if (FileReq == NULL) {
        return NULL;
    } else {
        if (AslRequestTags(FileReq,
                           ASLFR_TitleText, title,
                           ASLFR_InitialDrawer, initialdir,
                           ASLFR_InitialFile, initialfile,
                           ASLFR_Window, canvaslist->os->window,
                           TAG_END)) {
            strcpy(file_name, FileReq->fr_Drawer);
            AddPart(file_name, FileReq->fr_File, 1024);
        }
    }

    FreeAslRequest(FileReq);

    if (file_name[0] == '\0') {
        return NULL;
    }

    return file_name;
}

char *ui_filereq(const char *title, int template, char *initialdir, char *initialfile, 
                 image_contents_t *(*read_content)(const char *), int *autostart, char *resource_readonly)
{
    static char filename[1024];

#ifdef AMIGA_MORPHOS
    static const struct Hook NewVolumeHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)NewVolume, NULL };
    static const struct Hook NewFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)NewFile, NULL };
    static const struct Hook NewDirHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)NewDir, NULL };
    static const struct Hook NewParentDirHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)NewParentDir, NULL };
    static const struct Hook NewPopFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)NewPopFile, NULL };
    static const struct Hook NewCreateImageHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)NewCreateImage, NULL };
    static const struct Hook NewCreateTAPImageHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)NewCreateTAPImage, NULL };
#else
    static const struct Hook NewVolumeHook = { { NULL, NULL }, (VOID *)NewVolume, NULL, NULL };
    static const struct Hook NewFileHook = { { NULL, NULL }, (VOID *)NewFile, NULL, NULL };
    static const struct Hook NewDirHook = { { NULL, NULL }, (VOID *)NewDir, NULL, NULL };
    static const struct Hook NewParentDirHook = { { NULL, NULL }, (VOID *)NewParentDir, NULL, NULL };
    static const struct Hook NewPopFileHook = { { NULL, NULL }, (VOID *)NewPopFile, NULL, NULL };
    static const struct Hook NewCreateImageHook = { { NULL, NULL }, (VOID *)NewCreateImage, NULL, NULL };
    static const struct Hook NewCreateTAPImageHook = { { NULL, NULL }, (VOID *)NewCreateTAPImage, NULL, NULL };
#endif

    BOOL running = TRUE;
    ULONG signals;
    char *ret = NULL;
    int readonly = 0;

    read_content_func = read_content;
    if (initialdir == NULL) {
        initialdir = "";
    }
    if (initialfile == NULL) {
        initialfile = "";
    }

    if (template == IDD_NONE) { /* simple requester */
        return request_file(title, initialdir, initialfile);
    }

    if (resource_readonly != NULL) {
        resources_get_value(resource_readonly, (void *)&readonly);
    }

    app = CreateApp(title, template, resource_readonly, initialdir);
    if (app) {
        DoMethod(app->BT_ATTACH, MUIM_Notify, MUIA_Pressed, FALSE,
                 app->App, 2, MUIM_Application_ReturnID, RET_ATTACH);

        DoMethod(app->BT_CANCEL, MUIM_Notify, MUIA_Pressed, FALSE,
                 app->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(app->FILEREQ, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
                 app->FILEREQ, 3, MUIM_Set, MUIA_Window_Open, FALSE);

        DoMethod(app->FILEREQ, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
                 app->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(app->LV_VOLUMELIST, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
                 app->LV_VOLUMELIST, 2, MUIM_CallHook, &NewVolumeHook);

        DoMethod(app->LV_FILELIST, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
                 app->LV_FILELIST, 2, MUIM_CallHook, &NewFileHook);

        DoMethod(app->LV_FILELIST, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
                 app->LV_FILELIST, 2, MUIM_CallHook, &NewDirHook);

        DoMethod(app->BT_PARENT, MUIM_Notify, MUIA_Pressed, FALSE,
                 app->App, 2, MUIM_CallHook, &NewParentDirHook);

        DoMethod(app->LV_CONTENTS, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
                 app->App, 2, MUIM_Application_ReturnID, RET_AUTOSTART_ATTACH);

        DoMethod(app->STR_PA_FILEREQ, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
                 app->App, 2, MUIM_CallHook, &NewPopFileHook);

        DoMethod(app->BT_CREATEIMAGE, MUIM_Notify, MUIA_Pressed, FALSE,
                 app->App, 2, MUIM_CallHook, &NewCreateImageHook);

        DoMethod(app->BT_CREATETAPIMAGE, MUIM_Notify, MUIA_Pressed, FALSE,
                 app->App, 2, MUIM_CallHook, &NewCreateTAPImageHook);

        /* Initialize */

        set(app->CH_READONLY, MUIA_Selected, readonly);
        /* set(app->LV_FILELIST, MUIA_Dirlist_Directory, initialdir); */

        strcpy(filename, initialdir);
        AddPart(filename, initialfile, 1024);
        set(app->STR_PA_FILEREQ, MUIA_String_Contents, filename);

        while (running) {
            unsigned long retval = DoMethod(app->App,MUIM_Application_Input,&signals);

            switch (retval) {
                case MUIV_Application_ReturnID_Quit:
                    running = FALSE;
                    break;
                case RET_AUTOSTART_ATTACH:
                    get(app->LV_CONTENTS, MUIA_List_Active, (APTR)autostart);
                    /* fall through */
                case RET_ATTACH:
                    {
                        char *path;

                        get(app->STR_PA_FILEREQ, MUIA_String_Contents, (APTR)&path);
                        strcpy(filename, path);
                        get(app->CH_READONLY, MUIA_Selected, (APTR)&readonly);
                        ret = filename;
                        running = FALSE;
                    }
                    break;
                default:
                    break;
            }
            if (running && signals) {
                Wait(signals);
            }
        }
        DisposeApp(app);
    }  

    if (resource_readonly != NULL) {
        resources_set_value(resource_readonly, (resource_value_t)readonly);
    }

    return ret;
}
