// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef XMENU_H
#define XMENU_H
#include <Xm/RowColumn.h>
#include <Xm/CascadeBG.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/ToggleB.h>
#include <Xm/SeparatoG.h>

typedef struct _menu_overlay_visual {
  Visual         * overlayVisual;   // visual of overlay layer
  int              overlayDepth;    // depth of overlay layer
  Colormap         overlayColormap; // colormap for overlay of overlay layer
  XtCallbackProc   popupCallback;   // routine to install corrct colormap
  XtPointer        callback_data;   // client_data for popupCallback()
} MenuOverlayVisual;
  

typedef struct _menu_item {
  char              * label;         // the label for the item
  WidgetClass       * bclass;        // pushbutton, label, separator...
  char                mnemonic;      // mnemonic; NULL if none
  char              * accelerator;   // accelerator; NULL if none
  char              * accel_text;    // to be converted to compound string
  Boolean             togglestate;   // initial state for toggles
  XtCallbackProc      callback;      // routine to call; NULL if none
  XtPointer           callback_data; // client_data for callback()
  struct _menu_item * subitems;      // pullright menu items, if not NULL
} MenuItem;

Widget BuildMenu(Widget, int, char *, char, MenuOverlayVisual *, Boolean,
		 MenuItem *);

#endif // XMENU_H
