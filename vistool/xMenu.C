// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#include "xMenu.h"
#include <iostream>

Widget BuildMenu(const Widget parent, const int menu_type,
		 const char *const menu_title, const char menu_mnemonic,
		 const MenuOverlayVisual *const overlay,
		 const Boolean tear_off, const MenuItem *const items)
{
  Widget menu, cascade, widget;
  Arg args[4];
  int n = 0;

  if(overlay) {
    XtSetArg(args[n], XmNvisual,   overlay->overlayVisual);   n++;
    XtSetArg(args[n], XmNdepth,    overlay->overlayDepth);    n++;
    XtSetArg(args[n], XmNcolormap, overlay->overlayColormap); n++;
    // Hack to prevent transparent menu text when overlays are supported
    XtSetArg(args[n], XmNforeground, 1); n++;
  }

  if(menu_type == XmMENU_PULLDOWN)
    menu = XmCreatePulldownMenu(parent, "_pulldown", args, n);
  else
    menu = XmCreatePopupMenu(parent, "_popup", args, n);

  if(overlay)
    XtAddCallback(XtParent(menu), XmNpopupCallback, overlay->popupCallback,
		  overlay->callback_data);

  if(tear_off)
    XtVaSetValues(menu, XmNtearOffModel, XmTEAR_OFF_ENABLED, NULL);

  if(menu_type == XmMENU_PULLDOWN) {
    XmString str = XmStringCreateLocalized((char *)menu_title);
                                           // no const in def
    cascade = XtVaCreateManagedWidget(menu_title,
				      xmCascadeButtonGadgetClass, parent,
				      XmNsubMenuId, menu,
				      XmNlabelString, str,
				      XmNmnemonic, menu_mnemonic,
				      NULL);
    XmStringFree(str);
  }
  
  // Now add the menu items
  for(int i=0; items[i].label != NULL; i++) {
    // If subitems exist, create the pull-right menu by calling this
    // function recursively.  Since the function returns a cascade
    // button, the widget returned is used.
    if(items[i].subitems)
      widget = BuildMenu(menu, XmMENU_PULLDOWN, items[i].label,
			 items[i].mnemonic, overlay,
			 tear_off, items[i].subitems);
    else
      widget = XtVaCreateManagedWidget(items[i].label,
				       *items[i].bclass, menu,
				       NULL);
    // Whether the item is a real item or a cascade button with a
    // menu, it can still have a mnemonic.
    if(items[i].mnemonic)
      XtVaSetValues(widget, XmNmnemonic, items[i].mnemonic, NULL);
    // Any item can have an accelerator, except cascade menus.  But,
    // we don't worry about that; we know better in our declarations.
    if(items[i].accelerator) {
      XmString str = XmStringCreateLocalized((char *)items[i].accel_text);
                                             // no const in def
      XtVaSetValues(widget,
		    XmNaccelerator, items[i].accelerator,
		    XmNacceleratorText, str,
		    NULL);
      XmStringFree(str);
    }
    // Set state for any ToggleButton
    if(items[i].bclass == &xmToggleButtonWidgetClass ||
       items[i].bclass == &xmToggleButtonGadgetClass) {
      XtVaSetValues(widget, XmNvisibleWhenOff, True, NULL);
      XmToggleButtonSetState(widget, items[i].togglestate, False);
    }
    // Again, anyone can have a callback -- however, this is an
    // activate-callback.  This may not be appropriate for all items
    if(items[i].callback)
      XtAddCallback(widget,
		    (items[i].bclass == &xmToggleButtonWidgetClass ||
		     items[i].bclass == &xmToggleButtonGadgetClass) ?
		    XmNvalueChangedCallback : // ToggleButton class
		    XmNactivateCallback,      // PushButton class
		    (XtCallbackProc) items[i].callback,
		    items[i].callback_data);
  }
  return menu_type == XmMENU_POPUP ? menu : cascade;
}

