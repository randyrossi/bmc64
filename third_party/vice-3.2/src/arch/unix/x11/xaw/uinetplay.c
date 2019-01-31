/*
 * uinetplay.c - UI controls for netplay
 *
 * Written by
 *  Olaf Seibert <rhialto@falu.nl>
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
#include <string.h>

#ifdef HAVE_NETWORK

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/Command.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/Toggle.h>
#else
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Toggle.h>
#endif

#include <widgets/TextField.h>

#include "lib.h"
#include "log.h"
#include "network.h"
#include "resources.h"
#include "ui.h"
#include "uiapi.h"
#include "uiarch.h"
#include "uinetplay.h"
#include "util.h"
#include "x11ui.h"

static log_t np_log = LOG_ERR;

typedef struct np_controldata_s {
    const char *name;
    unsigned int s_mask;
    unsigned int c_mask;
} np_controldata_t;

typedef struct np_control_s {
    Widget s_cb;
    Widget c_cb;
} np_control_t;

#define NR_NPCONROLS 5
static np_controldata_t np_controldatas[] = {
    { N_("Keyboard"),
      NETWORK_CONTROL_KEYB,
      NETWORK_CONTROL_KEYB << NETWORK_CONTROL_CLIENTOFFSET },
    { N_("Joystick 1"),
      NETWORK_CONTROL_JOY1,
      NETWORK_CONTROL_JOY1 << NETWORK_CONTROL_CLIENTOFFSET },
    { N_("Joystick 2"),
      NETWORK_CONTROL_JOY2,
      NETWORK_CONTROL_JOY2 << NETWORK_CONTROL_CLIENTOFFSET },
    { N_("Devices"),
      NETWORK_CONTROL_DEVC,
      NETWORK_CONTROL_DEVC << NETWORK_CONTROL_CLIENTOFFSET },
    { N_("Settings"),
      NETWORK_CONTROL_RSRC,
      NETWORK_CONTROL_RSRC << NETWORK_CONTROL_CLIENTOFFSET },
    { 0 }
};

typedef struct {
    Widget netplay_dialog;
    Widget ports;
    Widget modelabel;
    Widget serverhosttext, serverporttext;
    Widget connecttotext;
    Widget disconnect;
    np_control_t controls[NR_NPCONROLS];
} dialog_data_t;

static dialog_data_t *g_dialogdata;

static void netplay_update_control_resources(dialog_data_t *dialogdata)
{
    int i;
    unsigned int control;

    resources_get_int("NetworkControl", (int *)&control);

    for (i = 0; i < NR_NPCONROLS; i++)
    {
        Boolean set;

        XtVaGetValues(dialogdata->controls[i].s_cb,
                            XtNstate, &set,
                            NULL);
        if (set) {
            control |=  np_controldatas[i].s_mask;
        } else {
            control &= ~np_controldatas[i].s_mask;
        }

        XtVaGetValues(dialogdata->controls[i].c_cb,
                            XtNstate, &set,
                            NULL);
        if (set) {
            control |=  np_controldatas[i].c_mask;
        } else {
            control &= ~np_controldatas[i].c_mask;
        }
    }
}

static void netplay_update_resources(dialog_data_t *dialogdata)
{
    const char *server_name;
    const char *server_bind_address;
    const char *porttext;
    long port;

    XtVaGetValues(dialogdata->serverhosttext, XtNstring, &server_bind_address, NULL);
    XtVaGetValues(dialogdata->serverporttext, XtNstring, &porttext, NULL);
    XtVaGetValues(dialogdata->connecttotext, XtNstring, &server_name, NULL);

    util_string_to_long(porttext, NULL, 10, &port);
    if (port < 1 || port > 0xFFFF) {
        ui_error(_("Invalid port number"));
        return;
    }
    resources_set_int("NetworkServerPort", (int)port);
    resources_set_string("NetworkServerName", server_name);
    resources_set_string("NetworkServerBindAddress", server_bind_address);

    netplay_update_control_resources(dialogdata);
}

static void netplay_update_control_gui(dialog_data_t *dialogdata)
{
    int i;
    unsigned int control;

    resources_get_int("NetworkControl", (int *)&control);
    for (i = 0; i < NR_NPCONROLS; i++)
    {
        int set;

        set = (control & np_controldatas[i].s_mask) ? True : False;
        XtVaSetValues(dialogdata->controls[i].s_cb,
                            XtNstate, set,
                            NULL);

        set = (control & np_controldatas[i].c_mask) ? True : False;
        XtVaSetValues(dialogdata->controls[i].c_cb,
                            XtNstate, set,
                            NULL);
    }
}

static void netplay_update_status(dialog_data_t *dialogdata)
{
    char *text = NULL;
    const char *server_name;
    const char *server_bind_address;
    int port;
    int sensitive_disconnect = True, sensitive_ports = True;
    char st[32];

    switch(network_get_mode()) {
        case NETWORK_IDLE:
            sensitive_disconnect = False;
            sensitive_ports = True;
            text = _("Idle");
            break;
        case NETWORK_SERVER:
            sensitive_disconnect = True;
            sensitive_ports = False;
            text = _("Server listening");
            break;
        case NETWORK_SERVER_CONNECTED:
            sensitive_disconnect = True;
            sensitive_ports = False;
            text = _("Connected server");
            break;
        case NETWORK_CLIENT:
            sensitive_disconnect = True;
            sensitive_ports = False;
            text = _("Connected client");
            break;
        default:
            break;
    }
    XtVaSetValues(dialogdata->modelabel, XtNlabel, text, NULL);
    XtSetSensitive(dialogdata->ports, sensitive_ports);
    XtSetSensitive(dialogdata->disconnect, sensitive_disconnect);

    resources_get_int("NetworkServerPort", &port);
    resources_get_string("NetworkServerName", &server_name);
    resources_get_string("NetworkServerBindAddress", &server_bind_address);
    snprintf(st, 32, "%d", port);
    XtVaSetValues(dialogdata->serverhosttext, XtNstring, server_bind_address, NULL);
    XtVaSetValues(dialogdata->serverporttext, XtNstring, st, NULL);
    XtVaSetValues(dialogdata->connecttotext, XtNstring, server_name, NULL);

    log_message(np_log, "Status: %s, Server: %s, Port: %d; server bind address: %s", text, server_name, port, server_bind_address);
    netplay_update_control_gui(dialogdata);
}

static void StartServerProc(Widget w, XtPointer client_data, XtPointer dummy)
{
    dialog_data_t *dialogdata = client_data;
    netplay_update_resources(dialogdata);
    if (network_start_server() < 0) {
        ui_error(_("Couldn't start netplay server."));
    }
    ui_popdown(dialogdata->netplay_dialog);
}

static void ConnectProc(Widget w, XtPointer client_data, XtPointer dummy)
{
    dialog_data_t *dialogdata = client_data;
    netplay_update_resources(dialogdata);
    if (network_connect_client() < 0) {
        ui_error(_("Couldn't connect client."));
    }
    ui_popdown(dialogdata->netplay_dialog);
}

static void DisconnectProc(Widget w, XtPointer client_data, XtPointer dummy)
{
    dialog_data_t *dialogdata = client_data;
    netplay_update_resources(dialogdata);
    network_disconnect();
    ui_popdown(dialogdata->netplay_dialog);
}

static void OkProc(Widget w, XtPointer client_data, XtPointer dummy)
{
    dialog_data_t *dialogdata = client_data;
    netplay_update_resources(dialogdata);
    ui_popdown(dialogdata->netplay_dialog);
}

static void CancelProc(Widget w, XtPointer client_data, XtPointer dummy)
{
    dialog_data_t *dialogdata = client_data;
    ui_popdown(dialogdata->netplay_dialog);
}

static Widget build_netplay_dialog(Widget parent, dialog_data_t *dialogdata)
{
    Widget shell;
    Widget form, ports;
    Widget toplabel;
    Widget modetextlabel, modelabel;
    Widget start, portlabel, serverhosttext, serverporttext;
    Widget connect, connecttotext;
    Widget disconnect, ok, cancel;
    Widget controllabel, controlform;

    /* popup window */
    shell = ui_create_transient_shell(parent, "Netplay settings");

    /* put a form inside it */
    form = XtVaCreateManagedWidget("netplayForm",
                                    formWidgetClass, shell,
                                    NULL);

    toplabel = XtVaCreateManagedWidget("topLabel",
                                    labelWidgetClass, form,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, _("Netplay Settings"),
                                    XtNborderWidth, 0,
                                    /* Constraints: */
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);

    /*
     * Wrap the host/port controls in a Form so they can be made
     * (in)sensitive as a whole.
     */
    ports = XtVaCreateManagedWidget("portsForm",
                                    formWidgetClass, form,
                                    XtNborderWidth, 1,
                                    /* Constraints: */
                                    XtNfromVert, toplabel,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);
    /* Row of the current mode */
    modetextlabel = XtVaCreateManagedWidget("modeTextLabel",
                                    labelWidgetClass, ports,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, _("Current mode:"),
                                    XtNborderWidth, 0,
                                    /* Constraints: */
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);

    modelabel = XtVaCreateManagedWidget("modeLabel",
                                    labelWidgetClass, ports,
                                    XtNjustify, XtJustifyLeft,
                                    XtNwidth, 256,
                                    XtNborderWidth, 0,
                                    /* Constraints: */
                                    XtNresizable, True,
                                    XtNfromHoriz, modetextlabel,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);

    /* Row of the server settings */
    start = XtVaCreateManagedWidget("start",
                                    commandWidgetClass, ports,
                                    XtNlabel, _("Start server"),
                                    /* Constraints: */
                                    XtNfromVert, modelabel,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);
    XtAddCallback(start, XtNcallback, StartServerProc, (XtPointer)dialogdata);

    portlabel = XtVaCreateManagedWidget("portLabel",
                                    labelWidgetClass, ports,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, _("TCP port:"),
                                    XtNborderWidth, 0,
                                    /* Constraints: */
                                    XtNfromVert, modelabel,
                                    XtNfromHoriz, start,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);

    serverhosttext = XtVaCreateManagedWidget("serverHost",
                                    textfieldWidgetClass, ports,
                                    XtNwidth, 10 * 10,
                                       /* Constraints: */
                                    XtNfromVert, modelabel,
                                    XtNfromHoriz, portlabel,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);

    serverporttext = XtVaCreateManagedWidget("serverPort",
                                    textfieldWidgetClass, ports,
                                    XtNwidth, 4 * 10,
                                    /* Constraints: */
                                    XtNfromVert, modelabel,
                                    XtNfromHoriz, serverhosttext,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);


    /* Row of the client settings */
    connect = XtVaCreateManagedWidget("connect",
                                    commandWidgetClass, ports,
                                    XtNlabel, _("Connect to"),
                                    /* Constraints: */
                                    XtNfromVert, start,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);
    XtAddCallback(connect, XtNcallback, ConnectProc, (XtPointer)dialogdata);

    connecttotext = XtVaCreateManagedWidget("connectTo",
                                    textfieldWidgetClass, ports,
                                    XtNwidth, 15 * 10,
                                    /* Constraints: */
                                    XtNfromVert, start,
                                    XtNfromHoriz, connect,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);
    /* End of "ports" Form */

    /* Put the block of checkboxes right next to it */
    controllabel = XtVaCreateManagedWidget("controlLabel",
                                    labelWidgetClass, form,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, _("Control"),
                                    XtNborderWidth, 0,
                                    /* Constraints: */
                                    XtNfromHoriz, ports,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);

    controlform = XtVaCreateManagedWidget("controlForm",
                                    formWidgetClass, form,
                                    XtNborderWidth, 1,
                                    /* Constraints: */
                                    XtNfromVert, controllabel,
                                    XtNfromHoriz, ports,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);

    {
        Widget fromVert = 0;
        Widget widestlabel = 0;
        Widget label1;
        int width = 0, height, labelheight;
        int widestsize = 0;
        int i;

        /* Get the height of another label which is hopefully the same */
        x11ui_get_widget_size(controllabel, &width, &labelheight);

#define DD              4       /* Default Distance */
#define BW              1       /* default BorderWidth */

        /* Put labels */
        for (i = 0; i < NR_NPCONROLS; i++) {
            const char *labelname = np_controldatas[i].name;
            Widget label;

            label = XtVaCreateManagedWidget(labelname,
                                    labelWidgetClass, controlform,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, labelname,
                                    XtNborderWidth, 0,
                                    /* Constraints: */
                                    XtNvertDistance,
                                            (i ? BW+DD+BW
                                               : DD+labelheight+DD+BW),
                                    XtNfromVert, fromVert,
                                    XtNleft, XawChainLeft,
                                    XtNright, XawChainLeft,
                                    NULL);

            fromVert = label;

            /* Determine which of the labels is the widest */
            width = 0;
            x11ui_get_widget_size(label, &width, &height);
            if (width > widestsize) {
                widestlabel = label;
                widestsize = width;
            }
        }

        label1 = XtVaCreateManagedWidget("server",
                                    labelWidgetClass, controlform,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, _("Server"),
                                    XtNborderWidth, 0,
                                    /* Constraints: */
                                    XtNfromHoriz, widestlabel,
                                    XtNhorizDistance, -2,
                                    XtNleft, XawChainLeft,
                                    XtNright, XawChainLeft,
                                    NULL);

        (void)XtVaCreateManagedWidget("client",
                                    labelWidgetClass, controlform,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, _("Client"),
                                    XtNborderWidth, 0,
                                    /* Constraints: */
                                    XtNfromHoriz, label1,
                                    XtNleft, XawChainLeft,
                                    XtNright, XawChainLeft,
                                    NULL);

        /* Put checkboxes */
        fromVert = label1;
        for (i = 0; i < NR_NPCONROLS; i++) {
            const char *labelname = lib_msprintf("cb%ds", i);
            Widget box1, box2;

            box1 = XtVaCreateManagedWidget(labelname,
                                    toggleWidgetClass, controlform,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, "  ",
                                    XtNborderWidth, BW,
                                    /* Constraints: */
                                    XtNfromVert, fromVert,
                                    XtNfromHoriz, widestlabel,
                                    XtNleft, XawChainLeft,
                                    XtNright, XawChainLeft,
                                    NULL);
            lib_free(labelname);

            labelname = lib_msprintf("cb%dc", i);
            box2 = XtVaCreateManagedWidget(labelname,
                                    toggleWidgetClass, controlform,
                                    XtNjustify, XtJustifyLeft,
                                    XtNlabel, "  ",
                                    XtNborderWidth, BW,
                                    /* Constraints: */
                                    XtNfromVert, fromVert,
                                    XtNfromHoriz, label1,
                                    XtNleft, XawChainLeft,
                                    XtNright, XawChainLeft,
                                    NULL);
            lib_free(labelname);
            fromVert = box1;

            dialogdata->controls[i].s_cb = box1;
            dialogdata->controls[i].c_cb = box2;
        }
    }

    /* End of block of checkboxes */

    /* Row with Disconnect */
    disconnect = XtVaCreateManagedWidget("disconnect",
                                    commandWidgetClass, form,
                                    XtNlabel, _("Disconnect"),
                                    /* Constraints: */
                                    XtNfromVert, ports,
                                    XtNtop, XawChainBottom,
                                    XtNbottom, XawChainBottom,
                                    NULL);
    XtAddCallback(disconnect, XtNcallback, DisconnectProc, (XtPointer)dialogdata);

    /* Bottom with Ok, Cancel */
    ok = XtVaCreateManagedWidget("ok",
                                    commandWidgetClass, form,
                                    XtNlabel, _("Ok"),
                                    /* Constraints: */
                                    XtNfromVert, disconnect,
                                    XtNtop, XawChainBottom,
                                    XtNbottom, XawChainBottom,
                                    NULL);
    XtAddCallback(ok, XtNcallback, OkProc, (XtPointer)dialogdata);

    cancel = XtVaCreateManagedWidget("cancel",
                                    commandWidgetClass, form,
                                    XtNlabel, _("Cancel"),
                                    /* Constraints: */
                                    XtNfromVert, disconnect,
                                    XtNfromHoriz, ok,
                                    XtNtop, XawChainBottom,
                                    XtNbottom, XawChainBottom,
                                    NULL);
    XtAddCallback(cancel, XtNcallback, CancelProc, (XtPointer)dialogdata);

    dialogdata->ports = ports;
    dialogdata->modelabel = modelabel;
    dialogdata->serverhosttext = serverhosttext;
    dialogdata->serverporttext = serverporttext;
    dialogdata->connecttotext = connecttotext;
    dialogdata->disconnect = disconnect;

    return shell;
}

void ui_netplay_dialog(void)
{
    if (!np_log) {
        np_log = log_open("Netplay");
    }

    if (!g_dialogdata) {
        g_dialogdata = lib_malloc(sizeof(dialog_data_t));
        g_dialogdata->netplay_dialog = build_netplay_dialog(_ui_top_level, g_dialogdata);
    }

    if (g_dialogdata && g_dialogdata->netplay_dialog) {
        netplay_update_status(g_dialogdata);
        ui_popup(g_dialogdata->netplay_dialog, "Netplay Dialog", TRUE);
    }
}

#endif /* HAVE_NETWORK */
