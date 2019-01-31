/*
 * mui.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#ifndef VICE_UI_MUI_H
#define VICE_UI_MUI_H

#include "vice.h"

#include "resources.h"
#include "private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MUIPROG_H
#define __USE_INLINE__
#include <exec/memory.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/asl.h>

#ifdef AMIGA_MORPHOS
#include <proto/alib.h> /* for DoMethod */
#endif

#ifdef AMIGA_M68K
#include <clib/alib_protos.h> /* for DoMethod */
#endif

#ifdef AMIGA_AROS
#define MUI_OBSOLETE
#endif

#include <proto/muimaster.h>

#if defined(AMIGA_M68K) || defined(AMIGA_AROS)
#include <libraries/mui.h>
#endif

#ifdef AMIGA_AROS
#include <libraries/asl.h>
#undef get
#undef set
#define get(obj, attr, store) GetAttr(attr, obj, (ULONG *)store)
#define set(obj, attr, value) SetAttrs(obj, attr, value, TAG_DONE)
#endif

#ifndef MAKE_ID
#define MAKE_ID(a, b, c, d) ((ULONG) (a) << 24 | (ULONG) (b) << 16 | (ULONG) (c) << 8 | (ULONG) (d))
#endif

#define CHECK(store, name)           \
    Child, GroupObject,              \
    MUIA_Group_Columns, 2,           \
    Child, store = CheckMark(FALSE), \
    Child, Label1(name),             \
    End,

#define CYCLE(store, name, entries) \
    Child, HGroup,                  \
    Child, TextObject,              \
    MUIA_Text_PreParse, "\033r",    \
    MUIA_Text_Contents, name,       \
    MUIA_Weight, 30,                \
    MUIA_InnerLeft, 0,              \
    MUIA_InnerRight, 0,             \
    End,                            \
    Child, store = CycleObject,     \
    MUIA_Cycle_Entries, entries,    \
    MUIA_Cycle_Active, 0,           \
    End,                            \
    End,

#define FILENAME(store, name, button)               \
    Child, HGroup,                                  \
    Child, TextObject,                              \
    MUIA_Text_PreParse, "\033r",                    \
    MUIA_Text_Contents, name,                       \
    MUIA_Weight, 30,                                \
    MUIA_InnerLeft, 0,                              \
    MUIA_InnerRight, 0,                             \
    End,                                            \
    Child, store = StringObject,                    \
    MUIA_Frame, MUIV_Frame_String,                  \
    MUIA_FrameTitle, name,                          \
    MUIA_String_MaxLen, 1024,                       \
    End,                                            \
    Child, button = TextObject,                     \
    ButtonFrame,                                    \
    MUIA_Background, MUII_ButtonBack,               \
    MUIA_Text_Contents, translate_text(IDS_BROWSE), \
    MUIA_Text_PreParse, "\033c",                    \
    MUIA_InputMode, MUIV_InputMode_RelVerify,       \
    End,                                            \
    End,

#define STRING(store, name, maxlen) \
    Child, HGroup,                  \
    Child, TextObject,              \
    MUIA_Text_PreParse, "\033r",    \
    MUIA_Text_Contents, name,       \
    MUIA_Weight, 30,                \
    MUIA_InnerLeft, 0,              \
    MUIA_InnerRight, 0,             \
    End,                            \
    Child, store = StringObject,    \
    MUIA_Frame, MUIV_Frame_String,  \
    MUIA_FrameTitle, name,          \
    MUIA_String_MaxLen, maxlen,     \
    End,                            \
    End,

#define NSTRING(store, name, accept, maxlen) \
    Child, HGroup,                           \
    Child, TextObject,                       \
    MUIA_Text_PreParse, "\033r",             \
    MUIA_Text_Contents, name,                \
    MUIA_Weight, 30,                         \
    MUIA_InnerLeft, 0,                       \
    MUIA_InnerRight, 0,                      \
    End,                                     \
    Child, store = StringObject,             \
    MUIA_Frame, MUIV_Frame_String,           \
    MUIA_FrameTitle, name,                   \
    MUIA_String_Accept, accept,              \
    MUIA_String_MaxLen, maxlen,              \
    End,                                     \
    End,

#define BUTTON(button, name)                  \
    Child, button = TextObject,               \
    ButtonFrame,                              \
    MUIA_Background, MUII_ButtonBack,         \
    MUIA_Text_Contents, name,                 \
    MUIA_Text_PreParse, "\033c",              \
    MUIA_InputMode, MUIV_InputMode_RelVerify, \
    End,

#define OK_CANCEL_BUTTON                            \
    Child, HGroup,                                  \
    Child, ok = TextObject,                         \
    ButtonFrame,                                    \
    MUIA_Background, MUII_ButtonBack,               \
    MUIA_Text_Contents, translate_text(IDMES_OK),   \
    MUIA_Text_PreParse, "\033c",                    \
    MUIA_InputMode, MUIV_InputMode_RelVerify,       \
    End,                                            \
    Child, cancel = TextObject,                     \
    ButtonFrame,                                    \
    MUIA_Background, MUII_ButtonBack,               \
    MUIA_Text_Contents, translate_text(IDS_CANCEL), \
    MUIA_Text_PreParse, "\033c",                    \
    MUIA_InputMode, MUIV_InputMode_RelVerify,       \
    End,                                            \
    End,

#ifdef AMIGA_MORPHOS
#define BROWSE(function, hook_function, object)                                    \
    static ULONG function(struct Hook *hook, Object *obj, APTR arg)                \
    {                                                                              \
        char *fname = NULL;                                                        \
                                                                                   \
        fname = BrowseFile(translate_text(IDS_SELECT_ROM_FILE), "#?", rom_canvas); \
                                                                                   \
        if (fname != NULL) {                                                       \
            set(object, MUIA_String_Contents, fname);                              \
        }                                                                          \
                                                                                   \
        return 0;                                                                  \
    }                                                                              \
                                                                                   \
    static const struct Hook hook_function = {{NULL, NULL}, (VOID *)HookEntry, (VOID *)function, NULL}
#else
#define BROWSE(function, hook_function, object)                                    \
    static ULONG function(struct Hook *hook, Object *obj, APTR arg)                \
    {                                                                              \
        char *fname = NULL;                                                        \
                                                                                   \
        fname = BrowseFile(translate_text(IDS_SELECT_ROM_FILE), "#?", rom_canvas); \
                                                                                   \
        if (fname != NULL) {                                                       \
            set(object, MUIA_String_Contents, fname);                              \
        }                                                                          \
                                                                                   \
        return 0;                                                                  \
    }                                                                              \
                                                                                   \
    static const struct Hook hook_function = { { NULL, NULL }, (VOID *)function, NULL, NULL }
#endif

#define MUI_TYPE_NONE      (0)
#define MUI_TYPE_RADIO     (1)
#define MUI_TYPE_CHECK     (2)
#define MUI_TYPE_CYCLE     (3)
#define MUI_TYPE_CYCLE_STR (4)
#define MUI_TYPE_CYCLE_SID (5)
#define MUI_TYPE_INTEGER   (6)
#define MUI_TYPE_FLOAT     (7)
#define MUI_TYPE_TEXT      (8)
#define MUI_TYPE_FILENAME  (9)

typedef struct {
    APTR object;
    int type;
    char *resource;
    char **strings;
    const int *values;
    char **string_choices;
} ui_to_from_t;

#define UI_END \
    {NULL, MUI_TYPE_NONE, NULL, NULL, NULL, NULL}

extern ui_to_from_t *ui_find_resource(ui_to_from_t *data, char *resource);

extern int mui_show_dialog(APTR gui, char *title, ui_to_from_t *data);

/* FIXME: remove */
extern void ui_get_from(ui_to_from_t *data);
extern void ui_get_to(ui_to_from_t *data);

/* new interface */

#define BTN_OK (32)

#define countof(array) (sizeof(array) / sizeof((array)[0]))

extern int mui_init(void);
extern APTR mui_get_app(void);
extern APTR mui_make_simple_window(APTR gui, char *title);
extern APTR mui_make_ok_cancel_window(APTR gui, char *title);
extern void mui_add_window(APTR window);
extern void mui_rem_window(APTR window);
extern int mui_run(void);
extern void mui_exit(void);
extern char *BrowseFile(char *select_text, char *pattern, video_canvas_t *canvas);
extern void ui_about(void);

#endif
