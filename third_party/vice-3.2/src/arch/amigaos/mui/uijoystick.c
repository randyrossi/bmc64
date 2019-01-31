/*
 * uijoystick.c
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

#include "vice.h"

#ifdef AMIGA_OS4
#include "mui.h"

#include "intl.h"
#include "joy.h"
#include "joyai.h"
#include "joystick.h"
#include "machine.h"
#include "resources.h"
#include "translate.h"
#include "uijoystick.h"
#include "userport_joystick.h"

static int ui_joystick_translate[] = {
    IDS_NONE,
    -1
};

static char *ui_joystick[] = {
    NULL,			/* "None" placeholder */
    "Numpad + Ctrl",
    "AI/Keyset A",
    "AI/Keyset B",
    NULL
};

static const int ui_joystick_values[] = {
    JOYDEV_NONE,
    JOYDEV_NUMPAD,
    JOYDEV_KEYSET1,
    JOYDEV_KEYSET2,
    -1
};

static int ui_joystick_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_joystick_enable[countof(ui_joystick_enable_translate)];

static const int ui_joystick_enable_values[] = {
    0,
    1,
    -1
};

static int ui_userport_c64_joystick_translate[] = {
    IDMS_CGA_USERPORT_JOY_ADAPTER,
    IDMS_PET_USERPORT_JOY_ADAPTER,
    IDMS_HUMMER_USERPORT_JOY_ADAPTER,
    IDMS_OEM_USERPORT_JOY_ADAPTER,
    IDMS_HIT_USERPORT_JOY_ADAPTER,
    IDMS_KINGSOFT_USERPORT_JOY_ADAPTER,
    IDMS_STARBYTE_USERPORT_JOY_ADAPTER,
    0
};

static char *ui_userport_c64_joystick[countof(ui_userport_c64_joystick_translate)];

static const int ui_userport_c64_joystick_values[] = {
    USERPORT_JOYSTICK_CGA,
    USERPORT_JOYSTICK_PET,
    USERPORT_JOYSTICK_HUMMER,
    USERPORT_JOYSTICK_OEM,
    USERPORT_JOYSTICK_HIT,
    USERPORT_JOYSTICK_KINGSOFT,
    USERPORT_JOYSTICK_STARBYTE,
    -1
};

static int ui_userport_joystick_translate[] = {
    IDMS_CGA_USERPORT_JOY_ADAPTER,
    IDMS_PET_USERPORT_JOY_ADAPTER,
    IDMS_HUMMER_USERPORT_JOY_ADAPTER,
    IDMS_OEM_USERPORT_JOY_ADAPTER,
    0
};

static char *ui_userport_joystick[countof(ui_userport_joystick_translate)];

static const int ui_userport_joystick_values[] = {
    USERPORT_JOYSTICK_CGA,
    USERPORT_JOYSTICK_PET,
    USERPORT_JOYSTICK_HUMMER,
    USERPORT_JOYSTICK_OEM,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "JoyDevice1", ui_joystick, ui_joystick_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "JoyDevice2", ui_joystick, ui_joystick_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "JoyDevice3", ui_joystick, ui_joystick_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "JoyDevice4", ui_joystick, ui_joystick_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "UserportJoy", ui_joystick_enable, ui_joystick_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "UserportJoyType", ui_userport_c64_joystick, ui_userport_c64_joystick_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "UserportJoyType", ui_joystick_enable, ui_joystick_enable_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_plus4[] = {
    { NULL, MUI_TYPE_CYCLE, "JoyDevice1", ui_joystick, ui_joystick_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "JoyDevice2", ui_joystick, ui_joystick_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "JoyDevice3", ui_joystick, ui_joystick_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidCartJoy", ui_joystick_enable, ui_joystick_enable_values, NULL },
    UI_END /* mandatory */
};

static ULONG BT0Click(struct Hook *hook, Object *obj, APTR arg)
{
    joyai_config(1);

    return 0;
}

static ULONG BT1Click(struct Hook *hook, Object *obj, APTR arg)
{
    joyai_config(2);

    return 0;
}

static APTR build_gui_c64(void)
{
    static const struct Hook BT0Hook = { { NULL, NULL }, (VOID *)BT0Click, NULL, NULL };
    static const struct Hook BT1Hook = { { NULL, NULL }, (VOID *)BT1Click, NULL, NULL };
    APTR BT0, BT1;

    APTR gui = GroupObject,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_1),
                     CYCLE(ui_to_from[0].object, "", ui_joystick)
                   End,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_2),
                     CYCLE(ui_to_from[1].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_USERPORT_ADAPTER),
                     CYCLE(ui_to_from[4].object, "", ui_joystick_enable)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_USERPORT_ADAPTER_TYPE),
                     CYCLE(ui_to_from[5].object, "", ui_userport_c64_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_IN_USERPORT_PORT_1),
                     CYCLE(ui_to_from[2].object, "", ui_joystick)
                   End,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_IN_USERPORT_PORT_2),
                     CYCLE(ui_to_from[3].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, BT0 = SimpleButton("Config AI/Keyset A"),
                   Child, BT1 = SimpleButton("Config AI/Keyset B"),
                 End,
               End;

    if (gui != NULL) {
        DoMethod(BT0, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT0, 2, MUIM_CallHook, &BT0Hook);

        DoMethod(BT1, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT1, 2, MUIM_CallHook, &BT1Hook);
    }

    return gui;
}

static APTR build_gui_c64dtv(void)
{
    static const struct Hook BT0Hook = { { NULL, NULL }, (VOID *)BT0Click, NULL, NULL };
    static const struct Hook BT1Hook = { { NULL, NULL }, (VOID *)BT1Click, NULL, NULL };
    APTR BT0, BT1;

    APTR gui = GroupObject,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_1),
                     CYCLE(ui_to_from[0].object, "", ui_joystick)
                   End,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_2),
                     CYCLE(ui_to_from[1].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_USERPORT_ADAPTER),
                     CYCLE(ui_to_from[4].object, "", ui_joystick_enable)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_IN_USERPORT_PORT_1),
                     CYCLE(ui_to_from[2].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, BT0 = SimpleButton("Config AI/Keyset A"),
                   Child, BT1 = SimpleButton("Config AI/Keyset B"),
                 End,
               End;

    if (gui != NULL) {
        DoMethod(BT0, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT0, 2, MUIM_CallHook, &BT0Hook);

        DoMethod(BT1, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT1, 2, MUIM_CallHook, &BT1Hook);
    }

    return gui;
}

static APTR build_gui_cbm5x0(void)
{
    static const struct Hook BT0Hook = { { NULL, NULL }, (VOID *)BT0Click, NULL, NULL };
    static const struct Hook BT1Hook = { { NULL, NULL }, (VOID *)BT1Click, NULL, NULL };
    APTR BT0, BT1;

    APTR gui = GroupObject,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_1),
                     CYCLE(ui_to_from[0].object, "", ui_joystick)
                   End,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_2),
                     CYCLE(ui_to_from[1].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, BT0 = SimpleButton("Config AI/Keyset A"),
                   Child, BT1 = SimpleButton("Config AI/Keyset B"),
                 End,
               End;

    if (gui != NULL) {
        DoMethod(BT0, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT0, 2, MUIM_CallHook, &BT0Hook);

        DoMethod(BT1, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT1, 2, MUIM_CallHook, &BT1Hook);
    }

    return gui;
}

static APTR build_gui_pet(void)
{
    static const struct Hook BT0Hook = { { NULL, NULL }, (VOID *)BT0Click, NULL, NULL };
    static const struct Hook BT1Hook = { { NULL, NULL }, (VOID *)BT1Click, NULL, NULL };
    APTR BT0, BT1;

    APTR gui = GroupObject,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_USERPORT_ADAPTER),
                     CYCLE(ui_to_from[4].object, "", ui_joystick_enable)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_USERPORT_ADAPTER_TYPE),
                     CYCLE(ui_to_from[6].object, "", ui_userport_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_IN_USERPORT_PORT_1),
                     CYCLE(ui_to_from[2].object, "", ui_joystick)
                   End,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_IN_USERPORT_PORT_2),
                     CYCLE(ui_to_from[3].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, BT0 = SimpleButton("Config AI/Keyset A"),
                   Child, BT1 = SimpleButton("Config AI/Keyset B"),
                 End,
               End;

    if (gui != NULL) {
        DoMethod(BT0, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT0, 2, MUIM_CallHook, &BT0Hook);

        DoMethod(BT1, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT1, 2, MUIM_CallHook, &BT1Hook);
    }

    return gui;
}

static APTR build_gui_vic20(void)
{
    static const struct Hook BT0Hook = { { NULL, NULL }, (VOID *)BT0Click, NULL, NULL };
    static const struct Hook BT1Hook = { { NULL, NULL }, (VOID *)BT1Click, NULL, NULL };
    APTR BT0, BT1;

    APTR gui = GroupObject,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_1),
                     CYCLE(ui_to_from[0].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_USERPORT_ADAPTER),
                     CYCLE(ui_to_from[4].object, "", ui_joystick_enable)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_USERPORT_ADAPTER_TYPE),
                     CYCLE(ui_to_from[6].object, "", ui_userport_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_IN_USERPORT_PORT_1),
                     CYCLE(ui_to_from[2].object, "", ui_joystick)
                   End,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_IN_USERPORT_PORT_2),
                     CYCLE(ui_to_from[3].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, BT0 = SimpleButton("Config AI/Keyset A"),
                   Child, BT1 = SimpleButton("Config AI/Keyset B"),
                 End,
               End;

    if (gui != NULL) {
        DoMethod(BT0, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT0, 2, MUIM_CallHook, &BT0Hook);

        DoMethod(BT1, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT1, 2, MUIM_CallHook, &BT1Hook);
    }

    return gui;
}

static APTR build_gui_plus4(void)
{
    static const struct Hook BT0Hook = { { NULL, NULL }, (VOID *)BT0Click, NULL, NULL };
    static const struct Hook BT1Hook = { { NULL, NULL }, (VOID *)BT1Click, NULL, NULL };
    APTR BT0, BT1;

    APTR gui = GroupObject,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_1),
                     CYCLE(ui_to_from_plus4[0].object, "", ui_joystick)
                   End,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_2),
                     CYCLE(ui_to_from_plus4[1].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_SIDCART_JOY),
                     CYCLE(ui_to_from_plus4[3].object, "", ui_joystick_enable)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, GroupObject,
                     MUIA_Frame, MUIV_Frame_Group,
                     MUIA_FrameTitle, translate_text(IDS_JOYSTICK_IN_SIDCART_PORT),
                     CYCLE(ui_to_from_plus4[2].object, "", ui_joystick)
                   End,
                 End,
                 Child, GroupObject,
                   MUIA_Group_Horiz, TRUE,
                   Child, BT0 = SimpleButton("Config AI/Keyset A"),
                   Child, BT1 = SimpleButton("Config AI/Keyset B"),
                 End,
               End;

    if (gui != NULL) {
        DoMethod(BT0, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT0, 2, MUIM_CallHook, &BT0Hook);

        DoMethod(BT1, MUIM_Notify, MUIA_Pressed, FALSE,
                 BT1, 2, MUIM_CallHook, &BT1Hook);
    }

    return gui;
}

void ui_joystick_settings_c64_dialog(void)
{
    intl_convert_mui_table(ui_joystick_translate, ui_joystick);
    intl_convert_mui_table(ui_joystick_enable_translate, ui_joystick_enable);
    intl_convert_mui_table(ui_userport_c64_joystick_translate, ui_userport_c64_joystick);
    mui_show_dialog(build_gui_c64(), translate_text(IDS_JOYSTICK_SETTINGS), ui_to_from);
}

void ui_joystick_settings_c64dtv_dialog(void)
{
    intl_convert_mui_table(ui_joystick_translate, ui_joystick);
    intl_convert_mui_table(ui_joystick_enable_translate, ui_joystick_enable);
    intl_convert_mui_table(ui_userport_joystick_translate, ui_userport_joystick);
    mui_show_dialog(build_gui_c64dtv(), translate_text(IDS_JOYSTICK_SETTINGS), ui_to_from);
}

void ui_joystick_settings_cbm5x0_dialog(void)
{
    intl_convert_mui_table(ui_joystick_translate, ui_joystick);
    intl_convert_mui_table(ui_joystick_enable_translate, ui_joystick_enable);
    intl_convert_mui_table(ui_userport_joystick_translate, ui_userport_joystick);
    mui_show_dialog(build_gui_cbm5x0(), translate_text(IDS_JOYSTICK_SETTINGS), ui_to_from);
}

void ui_joystick_settings_pet_dialog(void)
{
    intl_convert_mui_table(ui_joystick_translate, ui_joystick);
    intl_convert_mui_table(ui_joystick_enable_translate, ui_joystick_enable);
    intl_convert_mui_table(ui_userport_joystick_translate, ui_userport_joystick);
    mui_show_dialog(build_gui_pet(), translate_text(IDS_JOYSTICK_SETTINGS), ui_to_from);
}

void ui_joystick_settings_vic20_dialog(void)
{
    intl_convert_mui_table(ui_joystick_translate, ui_joystick);
    intl_convert_mui_table(ui_joystick_enable_translate, ui_joystick_enable);
    intl_convert_mui_table(ui_userport_joystick_translate, ui_userport_joystick);
    mui_show_dialog(build_gui_vic20(), translate_text(IDS_JOYSTICK_SETTINGS), ui_to_from);
}

void ui_joystick_settings_plus4_dialog(void)
{
    intl_convert_mui_table(ui_joystick_translate, ui_joystick);
    intl_convert_mui_table(ui_joystick_enable_translate, ui_joystick_enable);
    intl_convert_mui_table(ui_userport_joystick_translate, ui_userport_joystick);
    mui_show_dialog(build_gui_plus4(), translate_text(IDS_JOYSTICK_SETTINGS), ui_to_from_plus4);
}

void ui_joystick_swap_joystick(void)
{
    int device1;
    int device2;

    resources_get_value("JoyDevice1",(void *)&device1);
    resources_get_value("JoyDevice2",(void *)&device2);
    resources_set_value("JoyDevice1",(resource_value_t)device2);
    resources_set_value("JoyDevice2",(resource_value_t)device1);
}

void ui_joystick_swap_extra_joystick(void)
{
    int device3;
    int device4;

    resources_get_value("JoyDevice3",(void *)&device3);
    resources_get_value("JoyDevice4",(void *)&device4);
    resources_set_value("JoyDevice3",(resource_value_t)device4);
    resources_set_value("JoyDevice4",(resource_value_t)device3);
}
#endif
