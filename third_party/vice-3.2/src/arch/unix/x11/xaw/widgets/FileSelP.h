/****************************************************************************

	FileSelectorP.h

	This file is the private definition file for the File Selector
	Widget.

 ****************************************************************************/

/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 *
 * 	totty@cs.uiuc.edu
 *
 */

#ifndef VICE_FILESELECTORP_H_
#define VICE_FILESELECTORP_H_

#include "DirMgr.h"

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/Cardinals.h>
#else
#include <X11/Xaw/Cardinals.h>
#endif

#include <X11/CoreP.h>
#include <X11/Core.h>

/*---------------------------------------------------------------------------*

                            C O N S T A N T S

 *---------------------------------------------------------------------------*/

#define FS_DIR_STRING_SIZE 512

/* The following indices are used to find the appropriate widget
   data from the arrays that the data items are stored in                    */

#define FS_I_TITLE            0 /* Title Index */
#define FS_I_CUR_DIR_TEXT     1 /* Current Directory Label Index */
#define FS_I_PATH_LIST_TITLE  2 /* Path From Root List Title Index */
#define FS_I_FILE_LIST_TITLE  3 /* File List Title Index */
#define FS_I_PATH_LIST        4 /* Path From Root List Index */
#define FS_I_FILE_LIST        5 /* File List Index */
#define FS_I_OK_BUTTON        6 /* OK Button Index */
#define FS_I_CANCEL_BUTTON    7 /* Cancel Button Index */
#define FS_I_AUTOSTART_BUTTON 8 /* Go Up Directory Button */
#define FS_I_GOTO_BUTTON      9 /* Go To Directory Button */
#define FS_I_CUR_FILE_TEXT    10/* Filename Text Line */
#define FS_I_SELECT_BUTTON    11/* Select Filename Button */
#define FS_I_CONTENTS_BUTTON  12/* Directory Button */
#define FS_I_RO_TOGGLE        13/* Read-only toggle */
#define FS_NUM_CHILDREN       14/* Count Of Above Indices */

/*---------------------------------------------------------------------------*

      S T R U C T U R E   &   W I D G E T    A C C E S S    M A C R O S

 *---------------------------------------------------------------------------*/

#define BoxX(b) ((b)->x)
#define BoxY(b) ((b)->y)
#define BoxW(b) ((b)->w)
#define BoxH(b) ((b)->h)

#define ChildInfoCoords(ci) (&((ci)->coords))
#define ChildInfoWidget(ci) ((ci)->widget)

#define FSCorePart(w)      (&((w)->core))
#define FSCompositePart(w) (&((w)->composite))
#define FSMyPart(w)        (&((w)->fileSelector))

#define FSDirMgr(w)              (FSMyPart(w)->dir_mgr)
#define FSChildren(w)            (FSMyPart(w)->children)
#define FSCurrentDirectory(w)    (FSMyPart(w)->current_directory)
#define FSCurrentFile(w)         (FSMyPart(w)->current_file)
#define FSTitle(w)               (FSMyPart(w)->title)
#define FSPathList(w)            (FSMyPart(w)->path_list)
#define FSFileList(w)            (FSMyPart(w)->file_list)
#define FSSortMode(w)            (FSMyPart(w)->sort_mode)
#define FSPattern(w)             (FSMyPart(w)->pattern)
#define FSPathListCount(w)       (FSMyPart(w)->path_list_count)
#define FSFileListCount(w)       (FSMyPart(w)->file_list_count)
#define FSOkCallbacks(w)         (FSMyPart(w)->ok_button_callbacks)
#define FSCancelCallbacks(w)     (FSMyPart(w)->cancel_button_callbacks)
#define FSContentsCallbacks(w)   (FSMyPart(w)->contents_button_callbacks)
#define FSAutostartCallbacks(w)  (FSMyPart(w)->autostart_button_callbacks)
#define FSSelChangeCallbacks(w)  (FSMyPart(w)->sel_change_callbacks)
#define FSInternational(w)       (FSMyPart(w)->international)
#define FSShowOkButton(w)        (FSMyPart(w)->show_ok_button)
#define FSShowCancelButton(w)    (FSMyPart(w)->show_cancel_button)
#define FSShowContentsButton(w)  (FSMyPart(w)->show_contents_button)
#define FSShowAutostartButton(w) (FSMyPart(w)->show_autostart_button)
#define FSShowROToggle(w)        (FSMyPart(w)->show_ro_toggle)
#define FSFileSelected(w)        (FSMyPart(w)->file_selected)
#define FSBusyCursor(w)          (FSMyPart(w)->busy_cursor)
#define FSHandCursor(w)          (FSMyPart(w)->hand_cursor)
#define FSFlagLinks(w)           (FSMyPart(w)->flag_links)
#define FSCheckExistence(w)      (FSMyPart(w)->check_existence)
#define FSSelectDirectory(w)     (FSMyPart(w)->select_directory)
#define FSReadOnlySelected(w)    (FSMyPart(w)->read_only_selected)

#define FSNthChildInfo(w, n) (&(FSChildren(w)[n]))

#define FSNthCoords(w, n) (ChildInfoCoords(FSNthChildInfo(w,n)))
#define FSNthWidget(w, n) (ChildInfoWidget(FSNthChildInfo(w,n)))

#define CoreWidth(w)       ((w)->core.width)
#define CoreHeight(w)      ((w)->core.height)
#define CoreBorderWidth(w) ((w)->core.border_width)

/*---------------------------------------------------------------------------*

                      T Y P E    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

typedef struct {
    int x;
    int y;
    int w;
    int h;
} BOX;

/*---------------------------------------------------------------------------*

           W I D G E T    S T R U C T U R E    D E F I N I T I O N

 *---------------------------------------------------------------------------*/

typedef struct
{
    BOX coords;
    Widget widget;
} XfwfFileSelectorChildInfo;

typedef struct {
    XtCallbackList ok_button_callbacks;
    XtCallbackList cancel_button_callbacks;
    XtCallbackList contents_button_callbacks;
    XtCallbackList autostart_button_callbacks;
    XtCallbackList sel_change_callbacks;
    Boolean international;
    Boolean show_ok_button;
    Boolean show_cancel_button;
    Boolean show_contents_button;
    Boolean show_autostart_button;
    Boolean show_ro_toggle;
    Boolean read_only_selected;
    Boolean file_selected;
    Boolean flag_links;
    Boolean check_existence;
    Boolean select_directory;
    char *title;
    int sort_mode;
    char *pattern;
    DIRECTORY_MGR *dir_mgr;
    XfwfFileSelectorChildInfo children[FS_NUM_CHILDREN];
    char *current_directory;
    char *current_file;
    char **path_list;
    char **file_list;
    int path_list_count;
    int file_list_count;
    Cursor busy_cursor;
    Cursor hand_cursor;
} XfwfFileSelectorPart;

typedef struct _XfwfFileSelectorClassPart {
    int empty;
} XfwfFileSelectorClassPart;

typedef struct _XfwfFileSelectorClassRec {
    CoreClassPart core_class;
    CompositeClassPart composite_class;
    XfwfFileSelectorClassPart	fileSelector_class;
} XfwfFileSelectorClassRec;

	/* This Is What A Widget Instance Points To */

typedef struct _XfwfFileSelectorRec {
    CorePart core;
    CompositePart composite;
    XfwfFileSelectorPart fileSelector;
} XfwfFileSelectorRec;

extern XfwfFileSelectorClassRec xfwfFileSelectorClassRec;

#endif
