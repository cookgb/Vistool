// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/Frame.h>
#include <Xm/FileSB.h>
#include <X11/GLw/GLwMDrawA.h>

#include <iostream.h>
#include <unistd.h>

#include <map.h>

#include "xvistool.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Create main window and start event loop;
void main(int argc, char * argv[])
{
  xvt_mainwin * xvt = new xvt_mainwin(argc,argv);
  xvt->Loop();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void mw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
void mw_file_open(Widget, XtPointer, XtPointer);
void mw_file_quit(Widget, XtPointer, XtPointer);
void mw_help(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Creator for X11 version of main window.
xvt_mainwin::xvt_mainwin(int & argc, char ** argv)
  : vt_mainwin(), Open_Dialog(0), vi(0), overlayDepth(0),
    doubleBuffer(true)
{

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  XtSetLanguageProc(NULL,NULL,NULL);

  //--------------------------------------------------------------------------
  // Set fallback resources.
  String fallbackResources[] = {
    "*sgiMode: true",           // Try to enable Indigo Magic look & feel
    "*useSchemes: all",         // and SGI schemes.
    "xvistool.title: Visualization Tool",
    "*main.width: 350",
    "*main.height: 75",
    "*glxarea*width: 300",
    "*glxarea*height: 300",
    NULL
  };
  
  // Initialize toolkit and parse command line options.
  top_shell = XtVaAppInitialize(&app,"xvistool",NULL,0,&argc,argv,
				fallbackResources,NULL,NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Get the X11 display
  display = XtDisplay(top_shell);

  //--------------------------------------------------------------------------
  // OpenGL configuration string.
  int config[] = { GLX_DOUBLEBUFFER, GLX_RGBA,
		   GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
		   None };
  int * dblBuf = config  ;
  int *snglBuf = config+1;

  // Set the OpenGL visual
  if(!vi) {
    // Find an OpenGL-capable RGB visual
    vi = glXChooseVisual(display,DefaultScreen(display),dblBuf);
    if(!vi) {
      vi = glXChooseVisual(display,DefaultScreen(display),snglBuf);
      if(!vi) XtAppError(app,"no RGB visual");
      doubleBuffer = false;
    }
  }

  //--------------------------------------------------------------------------
  // Detect overlay support
//    {
//      int entries = 2; // need more than 2 colormap entries for reasonable menus.
//      for(int layer = 1; layer <=3; layer++) {
//        sovVisualInfo templ;
//        int nVisuals=0;
//        templ.layer = layer; templ.vinfo.screen = DefaultScreen(display);
//        sovVisualInfo * overlayVisuals =
//  	sovGetVisualInfo(display,
//  			 VisualScreenMask | VisualLayerMask,
//  			 &templ, &nVisuals);
//        if(overlayVisuals) {
//  	for(int i = 0; i < nVisuals; i++) {
//  	  if(overlayVisuals[i].vinfo.visual->map_entries > entries) {
//  	    overlayVisual = overlayVisuals[i].vinfo.visual;
//  	    overlayDepth  = overlayVisuals[i].vinfo.depth;
//  	    entries = overlayVisual->map_entries;
//  	  }
//  	}
//  	XFree(overlayVisuals);
//        }
//      }
//      if(overlayVisual) 
//        overlayColormap = XCreateColormap(display,DefaultRootWindow(display),
//  					overlayVisual, AllocNone);
//    }

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // main window contains MenuBar
  main_w = XtVaCreateManagedWidget("main",xmMainWindowWidgetClass,
				   top_shell,
				   XmNuserData,this,
				   NULL);

  //--------------------------------------------------------------------------
  // Create the MenuBar
  Arg args[2];
  XtSetArg(args[0], XmNuserData, this);
  menu_bar = XmCreateMenuBar(main_w, "menubar", args, 1);
  XtManageChild(menu_bar);

  //--------------------------------------------------------------------------
  // Set up arguments for menu overlay support if available
  int OLn=0;
  Arg OLargs[10];
  XtSetArg(OLargs[0], XmNuserData, this); OLn++;
  if(overlayVisual) {
    XtSetArg(OLargs[OLn], XmNvisual, overlayVisual); OLn++;
    XtSetArg(OLargs[OLn], XmNdepth, overlayDepth); OLn++;
    XtSetArg(OLargs[OLn], XmNcolormap, overlayColormap); OLn++;
  }

  //--------------------------------------------------------------------------
  // First menu is the File menu
  Widget menu_pane = XmCreatePulldownMenu(menu_bar, "menupane", OLargs, OLn);
  if(overlayVisual)
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) mw_ensurePulldownColormapInstalled, NULL);
  Widget btn;
  btn = XmCreatePushButton(menu_pane, "Open...", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) mw_file_open, NULL);
  XtManageChild(btn);
  btn = XmCreateSeparator(menu_pane, NULL, args, 1);
  XtManageChild(btn);
  btn = XmCreatePushButton(menu_pane, "Quit", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) mw_file_quit, NULL);
  XtManageChild(btn);
  XtSetArg(args[1], XmNsubMenuId, menu_pane);
  Widget cascade = XmCreateCascadeButton(menu_bar, "File", args, 2);
  XtManageChild(cascade);

  //--------------------------------------------------------------------------
  // Last menu is the Help menu
  menu_pane = XmCreatePulldownMenu(menu_bar, "menupane", OLargs, OLn);
  if(overlayVisual)
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) mw_ensurePulldownColormapInstalled, NULL);
  btn = XmCreatePushButton(menu_pane, "Help", NULL, 0);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) mw_help, NULL);
  XtManageChild(btn);
  XtSetArg(args[1], XmNsubMenuId, menu_pane);
  cascade = XmCreateCascadeButton(menu_bar, "Help", args, 2);
  // Tell the MenuBar which button is the help menu
  XtVaSetValues(menu_bar,XmNmenuHelpWidget,cascade,NULL);
  XtManageChild(cascade);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  XtRealizeWidget(top_shell);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Destructor for X11 main window
xvt_mainwin::~xvt_mainwin()
{
  // kill all of the X stuff
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Installation routine for colormap if overlay menus used
void mw_ensurePulldownColormapInstalled(Widget w, XtPointer client_data,
				     XtPointer call_data)
{
  // Get the xvt_mainwin.
  Widget lw = w;
  xvt_mainwin * mw = NULL;
  XtVaGetValues(w,XmNuserData,&mw,NULL);
  // Extra indirection if overlays are used
  if(!mw) {
    XtVaGetValues(XtParent(w),XmNuserData,&mw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "mw_ensurePulldownColormapInstalled" <<endl;
    lw = XtParent(w);
  }
  cout << "mw_ensurePulldownColormapInstalled" << endl;
  cout << "top_shell   : " << mw->top_shell   << endl;
  cout << "main_w      : " << mw->main_w      << endl;
  cout << "menu_bar    : " << mw->menu_bar    << endl;
  cout << "Open_Dialog : " << mw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  XInstallColormap(mw->display, mw->overlayColormap);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void mw_openfs_cb(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Callback routine for main window File->Open menu selection
void mw_file_open(Widget w, XtPointer client_data, XtPointer call_data)
{
  // Get the xvt_mainwin.
  Widget lw = w;
  xvt_mainwin * mw = NULL;
  // Extra indirection if overlays are used
  if(!mw) {
    XtVaGetValues(XtParent(w),XmNuserData,&mw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "mw_file_open" <<endl;
    lw = XtParent(w);
  }
  cout << "mw_file_open" << endl;
  cout << "top_shell   : " << mw->top_shell   << endl;
  cout << "main_w      : " << mw->main_w      << endl;
  cout << "menu_bar    : " << mw->menu_bar    << endl;
  cout << "Open_Dialog : " << mw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  if(!mw->Open_Dialog) {
    Arg args;
    XtSetArg(args, XmNuserData, mw);
    mw->Open_Dialog = XmCreateFileSelectionDialog(mw->top_shell,
						   "opendialog",
						   & args, 1);
    XtAddCallback(mw->Open_Dialog,XmNokCallback,mw_openfs_cb,NULL);
    XtAddCallback(mw->Open_Dialog,XmNcancelCallback,
		  (XtCallbackProc) XtUnmanageChild,NULL);
  }
  XtManageChild(mw->Open_Dialog);
  XtPopup(XtParent(mw->Open_Dialog),XtGrabNone);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window File->Quit menu selection
void mw_file_quit(Widget w, XtPointer client_data, XtPointer call_data)
{
  // Get the xvt_mainwin.
  Widget lw = w;
  xvt_mainwin * mw = NULL;
  XtVaGetValues(w,XmNuserData,&mw,NULL);
  // Extra indirection if overlays are used
  if(!mw) {
    XtVaGetValues(XtParent(w),XmNuserData,&mw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "mw_file_quit" <<endl;
    lw = XtParent(w);
  }
  cout << "mw_file_quit" << endl;
  cout << "top_shell   : " << mw->top_shell   << endl;
  cout << "main_w      : " << mw->main_w      << endl;
  cout << "menu_bar    : " << mw->menu_bar    << endl;
  cout << "Open_Dialog : " << mw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  delete mw;
  exit(0);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window Help->Help menu slection
void mw_help(Widget w, XtPointer client_data, XtPointer call_data)
{
  cout << "Selected Help" << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for open dialogue from main window File->Open 
void mw_openfs_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  // Get the xvt_mainwin.
  Widget lw = w;
  xvt_mainwin * mw = NULL;
  XtVaGetValues(w,XmNuserData,&mw,NULL);
  // Extra indirection if overlays are used
  if(!mw) {
    XtVaGetValues(XtParent(w),XmNuserData,&mw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "mw_openfs_cb" <<endl;
    lw = XtParent(w);
  }
  cout << "mw_openfs_cb" << endl;
  cout << "top_shell   : " << mw->top_shell   << endl;
  cout << "main_w      : " << mw->main_w      << endl;
  cout << "menu_bar    : " << mw->menu_bar    << endl;
  cout << "Open_Dialog : " << mw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  XmFileSelectionBoxCallbackStruct * fs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  char * file;

  if(fs) {
    if(!XmStringGetLtoR(fs->value,XmFONTLIST_DEFAULT_TAG,&file))
      return; // internal error
    // Create and register the new drawwin
    new xvt_drawwin(file,*mw);
    XtFree(file);
  }
  XtUnmanageChild(mw->Open_Dialog);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void dw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
void dw_file_open(Widget, XtPointer, XtPointer);
void dw_file_close(Widget, XtPointer, XtPointer);
void dw_help(Widget, XtPointer, XtPointer);
void activateMenu(Widget, XtPointer, XEvent *, Boolean *);
void dw_pulldownMenuUse(Widget, XtPointer, XtPointer);
void Button1DownAction(Widget, XEvent *, String *, Cardinal *);
void Button1MotionAction(Widget, XEvent *, String *, Cardinal *);
void Button2DownAction(Widget, XEvent *, String *, Cardinal *);
void Button2MotionAction(Widget, XEvent *, String *, Cardinal *);
//----------------------------------------------------------------------------
// Creator for X11 version of drawing window.
xvt_drawwin::xvt_drawwin(const char * filename, xvt_mainwin & mw)
  : vt_drawwin(filename,mw), xvt(mw)
{
  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Create an application shell for the draw window
  draw_shell = XtVaAppCreateShell("xvistool","drawshell",
				  topLevelShellWidgetClass,
				  xvt.display,
				  XmNtitle,filename,
				  NULL);

  static XtActionsRec dw_actionsTable[] = {
    {"Button1DownAction",Button1DownAction},
    {"Button1MotionAction",Button1MotionAction},
    {"Button2DownAction",Button2DownAction},
    {"Button2MotionAction",Button2MotionAction}
  };
  XtAppAddActions(xvt.app, dw_actionsTable, XtNumber(dw_actionsTable));

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // main window contains MenuBar
  main_w = XtVaCreateManagedWidget("drawwin",xmMainWindowWidgetClass,
				   draw_shell,
				   XmNuserData,this,
				   NULL);

  //--------------------------------------------------------------------------
  // Create the MenuBar
  Arg args[2];
  XtSetArg(args[0], XmNuserData, this);
  menu_bar = XmCreateMenuBar(main_w, "menubar", args, 1);
  XtManageChild(menu_bar);
  int OLn=0;
  Arg OLargs[10];
  XtSetArg(OLargs[0], XmNuserData, this); OLn++;
  if(xvt.overlayVisual) {
    XtSetArg(OLargs[OLn], XmNvisual, xvt.overlayVisual); OLn++;
    XtSetArg(OLargs[OLn], XmNdepth, xvt.overlayDepth); OLn++;
    XtSetArg(OLargs[OLn], XmNcolormap, xvt.overlayColormap); OLn++;
  }

  //--------------------------------------------------------------------------
  // First menu is the File menu
  Widget menu_pane = XmCreatePulldownMenu(menu_bar, "menupane", OLargs, OLn);
  if(xvt.overlayVisual)
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) dw_ensurePulldownColormapInstalled, NULL);
  Widget btn;
  btn = XmCreatePushButton(menu_pane, "Open...", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) dw_file_open, NULL);
  XtManageChild(btn);
  btn = XmCreateSeparator(menu_pane, NULL, args, 1);
  XtManageChild(btn);
  btn = XmCreatePushButton(menu_pane, "Close", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) dw_file_close,NULL);
  XtManageChild(btn);
  XtSetArg(args[1], XmNsubMenuId, menu_pane);
  Widget cascade = XmCreateCascadeButton(menu_bar, "File", args, 2);
  XtManageChild(cascade);

  //--------------------------------------------------------------------------
  // Last menu is the Help menu
  menu_pane = XmCreatePulldownMenu(menu_bar, "menupane", OLargs, OLn);
  if(xvt.overlayVisual)
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) dw_ensurePulldownColormapInstalled, NULL);
  btn = XmCreatePushButton(menu_pane, "Help", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) dw_help, NULL);
  XtManageChild(btn);
  XtSetArg(args[1], XmNsubMenuId, menu_pane);
  cascade = XmCreateCascadeButton(menu_bar, "Help", args, 2);
  // Tell the MenuBar which button is th help menu
  XtVaSetValues(menu_bar,XmNmenuHelpWidget,cascade,NULL);
  XtManageChild(cascade);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Frame area to contain OpenGL drawing area
  frame = XtVaCreateManagedWidget("frame",xmFrameWidgetClass,
				  main_w,
				  XmNuserData,this,
				  NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Create Popup menu for frame area of drawing window
  static XmButtonType buttonTypes[] = 
  {XmPUSHBUTTON, XmSEPARATOR, XmCHECKBUTTON};
  XmString buttonLabels[XtNumber(buttonTypes)];
  buttonLabels[0] = XmStringCreateLocalized("Reset");
  buttonLabels[1] = NULL;
  buttonLabels[2] = XmStringCreateLocalized("Animate");
  //--------------------------------------------------------------------------
  int pn = 0;
  Arg pargs[10];
  XtSetArg(pargs[pn], XmNbuttonCount, XtNumber(buttonTypes)); pn++;
  XtSetArg(pargs[pn], XmNbuttons, buttonLabels); pn++;
  XtSetArg(pargs[pn], XmNbuttonType, buttonTypes); pn++;
  XtSetArg(pargs[pn], XmNbuttonSet, 2); pn++;
  XtSetArg(pargs[pn], XmNsimpleCallback, dw_pulldownMenuUse); pn++;
  XtSetArg(pargs[pn], XmNuserData, this); pn++;
  if(xvt.overlayVisual) {
    XtSetArg(pargs[pn], XmNvisual, xvt.overlayVisual); pn++;
    XtSetArg(pargs[pn], XmNdepth, xvt.overlayDepth); pn++;
    XtSetArg(pargs[pn], XmNcolormap, xvt.overlayColormap); pn++;
  }
  popup = XmCreateSimplePopupMenu(frame, "popup", pargs, pn);
  XtAddEventHandler(frame, ButtonPressMask, False, activateMenu, &popup);
  for(int ifree=0; ifree<3; ifree++) XmStringFree(buttonLabels[ifree]);
  XtVaGetValues(popup, XmNchildren, &popuplist, NULL);
  XmToggleButtonSetState(popuplist[2], True, False);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // OpenGL drawing area
  glx_area = XtVaCreateManagedWidget("glxarea",glwMDrawingAreaWidgetClass,
				     frame,
				     GLwNvisualInfo,xvt.vi,
				     XmNuserData,this,
				     NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Set up mouse button actions
  static char * glxareaTranslations =
    "#override\n\
    <Btn1Down>:Button1DownAction()\n\
    <Btn1Motion>:Button1MotionAction()\n\
    <Btn2Down>:Button2DownAction()\n\
    <Btn2Motion>:Button2MotionAction()\n";
  XtTranslations trans = XtParseTranslationTable(glxareaTranslations);
  XtOverrideTranslations(glx_area, trans);


  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  XmMainWindowSetAreas(main_w,menu_bar,NULL,NULL,NULL,frame);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  XtRealizeWidget(draw_shell);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Destructor for X11 drawing window
xvt_drawwin::~xvt_drawwin()
{
  // *** NOTE *** We are destroying a Shell and not a Widget ????
  // *** Mesa error message ***
  //     "Warning: XtRemoveGrab asked to remove a widget not on the list"
  // cout << "in xvt_drawwin destructor" << endl;
  XtDestroyWidget(draw_shell);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Installation routine for colormap if overlay menus used
void dw_ensurePulldownColormapInstalled(Widget w, XtPointer client_data,
					XtPointer call_data)
{
  // Get the xvt_mainwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "dw_ensurePulldownColormapInstalled" <<endl;
    lw = XtParent(w);
  }
  cout << "dw_ensurePulldownColormapInstalled" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  XInstallColormap(dw->xvt.display, dw->xvt.overlayColormap);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//  void dw_openfs_cb(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Callback routine for drawing window File->Open menu selection
void dw_file_open(Widget w, XtPointer client_data, XtPointer call_data)
{
  // Get the xvt_drawwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "dw_file_open" <<endl;
    lw = XtParent(w);
  }
  cout << "dw_file_open" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

//        if(!xvt->Open_Dialog) {
//  	xvt->Open_Dialog = XmCreateFileSelectionDialog(xvt->top_shell,
//  						       "opendialog",
//  						       NULL,0);
//  	XtAddCallback(xvt->Open_Dialog,XmNokCallback,openfs_cb,NULL);
//  	XtAddCallback(xvt->Open_Dialog,XmNcancelCallback,
//  		      (XtCallbackProc) XtUnmanageChild,NULL);
//        }
//        XtManageChild(xvt->Open_Dialog);
//        XtPopup(XtParent(xvt->Open_Dialog),XtGrabNone);
  cout << "Selected Open... in a draw window" << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window File->Close menu selection
void dw_file_close(Widget w, XtPointer client_data, XtPointer call_data)
{
  // Get the xvt_drawwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "dw_file_close" <<endl;
    lw = XtParent(w);
  }
  cout << "dw_file_close" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  if(dw) {
    dw->close();
  } else {
    cerr << "Widge has no xvt_drawwin resource in drawwin close." << endl;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for drawing window Help->Help menu slection
void dw_help(Widget w, XtPointer client_data, XtPointer call_data)
{
  cout << "Selected Help in a draw window" << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for open dialogue from drawing window File->Open 
//  void dw_openfs_cb(Widget w, XtPointer client_data, XtPointer call_data)
//  {
//    XmFileSelectionBoxCallbackStruct * fs = 
//      (XmFileSelectionBoxCallbackStruct *) call_data;

//    char * file;

//    if(fs) {
//      if(!XmStringGetLtoR(fs->value,XmFONTLIST_DEFAULT_TAG,&file))
//        return; // internal error
//      cout << "Opening " << file << endl;
//      XtFree(file);
//    }
//    XtUnmanageChild(xvt->Open_Dialog);
//  }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void activateMenu(Widget w, XtPointer client_data, XEvent *event,
		  Boolean *cont)
{
  // Get the xvt_drawwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "activateMenu" <<endl;
    lw = XtParent(w);
  }
  cout << "activateMenu" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  if(dw->xvt.overlayVisual)
    XInstallColormap(dw->xvt.display, dw->xvt.overlayColormap);
  XmMenuPosition(dw->popup, &event->xbutton);
  XtManageChild(dw->popup);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_pulldownMenuUse(Widget w, XtPointer client_data, XtPointer call_data)
{
  // Get the xvt_drawwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "dw_pulldownMenuUse" <<endl;
    lw = XtParent(w);
  }
  cout << "dw_pulldownMenuUse" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  switch((int) client_data) {
  case 0:
    cout << "Reset popup button selected." << endl;
    break;
  case 1:
    cout << "Animate popup button selected." << endl;
    break;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button1DownAction(Widget w, XEvent * event, String * params, 
		       Cardinal * num_params)
{
  // Get the xvt_drawwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "Button1DownAction" <<endl;
    lw = XtParent(w);
  }
  cout << "Button1DownAction" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw          << endl;

  const int x = event->xbutton.x;
  const int y = event->xbutton.y;

  cout << "Button 1 Down: \n"
       << "Mouse at (" << x << "," << y << ")\n"
       << "String : " << params << "\n"
       << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button1MotionAction(Widget w, XEvent * event, String * params, 
			 Cardinal * num_params)
{
  // Get the xvt_drawwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "Button1MotionAction" <<endl;
    lw = XtParent(w);
  }
  cout << "Button1MotionAction" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  const int x = event->xbutton.x;
  const int y = event->xbutton.y;

  cout << "Button 1 Motion: \n"
       << "Mouse at (" << x << "," << y << ")\n"
       << "String : " << params << "\n"
       << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button2DownAction(Widget w, XEvent * event, String * params, 
		       Cardinal * num_params)
{
  // Get the xvt_drawwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "Button2DownAction" <<endl;
  }
  cout << "Button2DownAction" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  const int x = event->xbutton.x;
  const int y = event->xbutton.y;

  cout << "Button 2 Down: \n"
       << "Mouse at (" << x << "," << y << ")\n"
       << "String : " << params << "\n"
       << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button2MotionAction(Widget w, XEvent * event, String * params, 
			 Cardinal * num_params)
{
  // Get the xvt_drawwin.
  Widget lw = w;
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  // Extra indirection if overlays are used
  if(!dw) {
    XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);
    cout << "**** Getting Parent Widget in "
	 << "Button2MotionAction" <<endl;
    lw = XtParent(w);
  }
  cout << "Button2MotionAction" << endl;
  cout << "draw_shell  : " << dw->draw_shell  << endl;
  cout << "main_w      : " << dw->main_w      << endl;
  cout << "frame       : " << dw->frame       << endl;
  cout << "glx_area    : " << dw->glx_area    << endl;
  cout << "menu_bar    : " << dw->menu_bar    << endl;
  cout << "popup       : " << dw->popup       << endl;
  cout << "popuplist   : " << dw->popuplist   << endl;
  cout << "Open_Dialog : " << dw->Open_Dialog << endl;
  cout << "Widget      : " << lw              << endl;

  const int x = event->xbutton.x;
  const int y = event->xbutton.y;

  cout << "Button 2 Motion: \n"
       << "Mouse at (" << x << "," << y << ")\n"
       << "String : " << params << "\n"
       << "Number of params : " << *num_params << endl;
}
