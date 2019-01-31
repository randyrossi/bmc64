/*
 * ui-cmdline-options.c
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
 *  Andreas Boose <viceteam@t-online.de>
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

#include <stdio.h>

#include "cmdline.h"
#include "machine.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"

static const cmdline_option_t common_cmdline_options[] = {
    { "-htmlbrowser", SET_RESOURCE, -1,
      NULL, NULL, "HTMLBrowserCommand", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      N_("<Command>"), N_("Specify an HTML browser for the on-line help") },
    { "-saveres", SET_RESOURCE, 0,
      NULL, NULL, "SaveResourcesOnExit", (void *)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Save settings on exit") },
    { "+confirmexit", SET_RESOURCE, 0,
      NULL, NULL, "ConfirmOnExit", (void *)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Never confirm quitting VICE") },
    { "-confirmexit", SET_RESOURCE, 0,
      NULL, NULL, "ConfirmOnExit", (void *)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Confirm quitting VICE") },
    { "+saveres", SET_RESOURCE, 0,
      NULL, NULL, "SaveResourcesOnExit", (void *)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Never save settings on exit") },
    CMDLINE_LIST_END
};

static const cmdline_option_t cmdline_options[] = {
    { "-colormap", SET_RESOURCE, 0,
      NULL, NULL, "PrivateColormap", (void *)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Install a private colormap") },
    { "+colormap", SET_RESOURCE, 0,
      NULL, NULL, "PrivateColormap", (void *)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Use the default colormap") },
    { "-displaydepth", SET_RESOURCE, -1,
      NULL, NULL, "DisplayDepth", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      N_("<value>"), N_("Specify X display depth (1..32)") },
#if defined (USE_XF86_EXTENSIONS) && \
    (defined(USE_XF86_VIDMODE_EXT) || defined (HAVE_XRANDR))
    { "-fullscreen", SET_RESOURCE, 0,
      NULL, NULL, "UseFullscreen", (void *)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Enable fullscreen") },
    { "+fullscreen", SET_RESOURCE, 0,
      NULL, NULL, "UseFullscreen", (void *)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Disable fullscreen") },
#endif
    CMDLINE_LIST_END
};

int ui_cmdline_options_init(void)
{
    if (machine_class != VICE_MACHINE_VSID) {
        if (cmdline_register_options(cmdline_options) < 0) {
            return -1;
        }
    }
    return cmdline_register_options(common_cmdline_options);
}
