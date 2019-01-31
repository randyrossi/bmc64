/*****************************************************************************

	ScrolledList.h

	This file contains the user includes for the ScrolledList widget.

******************************************************************************/

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

#ifndef VICE_SCROLLEDLIST_H_
#define VICE_SCROLLEDLIST_H_

extern WidgetClass xfwfScrolledListWidgetClass;

typedef struct _XfwfScrolledListClassRec *XfwfScrolledListWidgetClass;
typedef struct _XfwfScrolledListRec *XfwfScrolledListWidget;

typedef struct {
    Widget multilist;	/* The list widget clicked on */
    String string;		/* The name of the item clicked on */
    int index;		/* The index of the item clicked on */
    Boolean highlighted;	/* Are any items highlighted? */
} XfwfScrolledListReturnStruct;

#define XtNlist          "list"
#define XtNnumberStrings "numberStrings"

#ifndef XtNsensitiveArray
#define XtNsensitiveArray "sensitiveArray"
#endif

/*===========================================================================*

                    E X P O R T E D    F U N C T I O N S

 *===========================================================================*/

extern void XfwfScrolledListSetList(Widget w, char **newlist, int items, int resize, Boolean *sensitive_array);
extern void XfwfScrolledListUnhighlightAll(Widget w);
extern void XfwfScrolledListHighlightItem(Widget w, int item_index);
extern void XfwfScrolledListShowItem(Widget w, int item_index);
extern XfwfScrolledListReturnStruct *XfwfScrolledListGetHighlighted(XfwfScrolledListWidget sw);
extern Boolean XfwfScrolledListIsHighlighted(Widget w, int item_index);
extern Boolean XfwfScrolledListGetItem(Widget w, int item_index, String *str_ptr, Boolean *high_ptr, Boolean *sens_ptr);

#endif
