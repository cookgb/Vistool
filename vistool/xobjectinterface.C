// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//
// X11/Motif - C++ interface routines.
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <Xm/RowColumn.h>
#include <Xm/FileSB.h>
#include <X11/GLw/GLwMDrawA.h>

#include "xvistool.h"

#define ANIMATETIME 20

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Installation routine for colormap if overlay menus used
void ensurePulldownColormapInstalled(Widget w, XtPointer client_data,
				     XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  XInstallColormap(mw->Xdisplay(), mw->XoverlayCM());
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void mw_openfs_cb(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Callback routine for main window File->Open menu selection
void mw_file_open(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;

  if(!mw->OpenDialog()) {
    mw->Open_Dialog = XmCreateFileSelectionDialog(mw->top_shell, "opendialog",
						  NULL, 0);
    XtAddCallback(mw->Open_Dialog, XmNokCallback, mw_openfs_cb, mw);
    XtAddCallback(mw->Open_Dialog, XmNcancelCallback,
		  (XtCallbackProc) XtUnmanageChild, NULL);
  }
  XtManageChild(mw->OpenDialog());
  XtPopup(XtParent(mw->OpenDialog()),XtGrabNone);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for open dialogue from main window File->Open 
void mw_openfs_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;

  XmFileSelectionBoxCallbackStruct * fs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  char * file;

  if(fs) {
    if(!XmStringGetLtoR(fs->value,XmFONTLIST_DEFAULT_TAG,&file))
      return; // internal error
    // Create and register the new drawwin
    xvt_drawwin * dw = new xvt_drawwin(file,*mw);
    if(!(mw->ImportFile(file,*dw))) dw->close();
    XtFree(file);
  }
  XtUnmanageChild(mw->OpenDialog());
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window File->Quit menu selection
void mw_file_quit(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  delete mw;
  exit(0);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window Help->Help menu slection
void mw_help(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_mainwin * mw = (xvt_mainwin *) client_data;
  cout << "Selected Help" << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void handleAnimate(xvt_mainwin *, XtIntervalId *);
//----------------------------------------------------------------------------
// Installation routine for colormap if overlay menus used
void mapStateChanged(Widget w, XtPointer client_data, XEvent *event,
		     Boolean * cont)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  switch (event->type) {
  case MapNotify:
    if(dw->animate) {
      if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount)
	dw->xmvt.animateID =
	  XtAppAddTimeOut(dw->xmvt.Xapp(), ANIMATETIME,
			  (XtTimerCallbackProc) handleAnimate, &dw->xmvt);
      dw->xmvt.animateHiddenCount--;
    }
    dw->visible = true;
    break;
  case UnmapNotify:
    if(dw->animate) {
      dw->xmvt.animateHiddenCount++;
      if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount) {
	XtRemoveTimeOut(dw->xmvt.animateID);
      }
    }
    dw->visible = false;
    break;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void handleAnimate(xvt_mainwin *, XtIntervalId *);
//----------------------------------------------------------------------------
// 
void dw_popup_animate(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  dw->animate = !dw->animate;
  if(dw->animate) {
    if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount)
      dw->xmvt.animateID =
	XtAppAddTimeOut(dw->xmvt.Xapp(), ANIMATETIME,
			(XtTimerCallbackProc) handleAnimate, &dw->xmvt);
    dw->xmvt.animateCount++;
  } else {
    dw->xmvt.animateCount--;
    if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount)
      XtRemoveTimeOut(dw->xmvt.animateID);
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void handleAnimate(xvt_mainwin * xmvt, XtIntervalId * id)
{
  xmvt->incrementAnimation();
  xmvt->animateID = XtAppAddTimeOut(xmvt->Xapp(), ANIMATETIME,
				    (XtTimerCallbackProc) handleAnimate, xmvt);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window File->Close menu selection
void dw_file_close(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  if(dw) {
    dw->close();
  } else {
    cerr << "client data not set in drawwin close." << endl;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//  void dw_openfs_cb(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Callback routine for drawing window File->Open menu selection
void dw_file_open(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_drawwin * dw = (xvt_drawwin *) client_data;
  cout << "Selected Open... in a draw window" << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for drawing window Help->Help menu slection
void dw_help(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_drawwin * dw = (xvt_drawwin *) client_data;
  cout << "Selected Help in a draw window" << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void activatePopup(Widget w, XtPointer client_data, XEvent *event,
		   Boolean * cont)
{
  Widget popup = (Widget) client_data;
  XButtonPressedEvent *bevent = (XButtonPressedEvent *) event;
  if(bevent->button != 3) return;
  XmMenuPosition(popup, bevent);
  XtManageChild(popup);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_popup_reset(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_drawwin * dw = (xvt_drawwin *) client_data;
  cout << "Reset popup button selected." << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button1DownAction(Widget w, XEvent * event, String * params, 
		       Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 1 Down: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button1MotionAction(Widget w, XEvent * event, String * params, 
			 Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 1 Motion: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button1UpAction(Widget w, XEvent * event, String * params, 
		     Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 1 Up: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button2DownAction(Widget w, XEvent * event, String * params, 
		       Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 2 Down: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button2MotionAction(Widget w, XEvent * event, String * params, 
			 Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 2 Motion: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button2UpAction(Widget w, XEvent * event, String * params, 
		     Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 2 Up: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_draw(Widget w, XtPointer data, XtPointer callData)
{
  xvt_drawwin * dw = (xvt_drawwin *) data;

  dw->postRedisplay();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_resize(Widget w, XtPointer data, XtPointer callData)
{
  xvt_drawwin * dw = (xvt_drawwin *) data;
  GLwDrawingAreaCallbackStruct * resize =
    (GLwDrawingAreaCallbackStruct *) callData;

  glXWaitX(); // Wait until X events are processed.
  dw->resize(resize->width,resize->height);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_init(Widget w, XtPointer data, XtPointer callData)
{
  xvt_drawwin * dw = (xvt_drawwin *) data;
  
  dw->init(dw->wWidth(), dw->wHeight());
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void handleRedisplay(xvt_mainwin * xmvt)
{
  xmvt->redisplay();
}






