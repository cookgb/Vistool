// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#include <Xm/Protocols.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleBG.h>
#include <Xm/ToggleB.h>
#include <Xm/SeparatoG.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/Frame.h>
#include <Xm/FileSB.h>
#include <X11/GLw/GLwMDrawA.h>

#include <GL/glx.h>

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
  : vt_mainwin(), Open_Dialog(0), vi(0), overlayDepth(0), doubleBuffer(true),
    animateCount(0), animateHiddenCount(0),animateID(0)
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
  {
    int entries = 2; // need more than 2 colormap entries for reasonable menus.
    for(int layer = 1; layer <=3; layer++) {
      sovVisualInfo templ;
      int nVisuals=0;
      templ.layer = layer; templ.vinfo.screen = DefaultScreen(display);
      sovVisualInfo * overlayVisuals =
	sovGetVisualInfo(display,
			 VisualScreenMask | VisualLayerMask,
			 &templ, &nVisuals);
      if(overlayVisuals) {
	for(int i = 0; i < nVisuals; i++) {
	  if(overlayVisuals[i].vinfo.visual->map_entries > entries) {
	    overlayVisual = overlayVisuals[i].vinfo.visual;
	    overlayDepth  = overlayVisuals[i].vinfo.depth;
	    entries = overlayVisual->map_entries;
	  }
	}
	XFree(overlayVisuals);
      }
    }
    if(overlayVisual) 
      overlayColormap = XCreateColormap(display,DefaultRootWindow(display),
					overlayVisual, AllocNone);
  }

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
		  (XtCallbackProc) mw_ensurePulldownColormapInstalled, this);
  Widget btn;
  btn = XmCreatePushButtonGadget(menu_pane, "Open...", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) mw_file_open, this);
  XtManageChild(btn);
  btn = XmCreateSeparatorGadget(menu_pane, NULL, args, 1);
  XtManageChild(btn);
  btn = XmCreatePushButtonGadget(menu_pane, "Quit", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) mw_file_quit, this);
  XtManageChild(btn);
  XtSetArg(args[1], XmNsubMenuId, menu_pane);
  Widget cascade = XmCreateCascadeButton(menu_bar, "File", args, 2);
  XtManageChild(cascade);

  //--------------------------------------------------------------------------
  // Last menu is the Help menu
  menu_pane = XmCreatePulldownMenu(menu_bar, "menupane", OLargs, OLn);
  if(overlayVisual)
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) mw_ensurePulldownColormapInstalled, this);
  btn = XmCreatePushButtonGadget(menu_pane, "Help", NULL, 0);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) mw_help, this);
  XtManageChild(btn);
  XtSetArg(args[1], XmNsubMenuId, menu_pane);
  cascade = XmCreateCascadeButton(menu_bar, "Help", args, 2);
  // Tell the MenuBar which button is the help menu
  XtVaSetValues(menu_bar, XmNmenuHelpWidget, cascade, NULL);
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
void mapStateChanged(Widget, XtPointer, XEvent *, Boolean *);
void dw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
void dw_file_open(Widget, XtPointer, XtPointer);
void dw_file_close(Widget, XtPointer, XtPointer);
void dw_help(Widget, XtPointer, XtPointer);
void activatePopup(Widget, XtPointer, XEvent *, Boolean *);
void dw_popup_reset(Widget, XtPointer, XtPointer);
void dw_popup_animate(Widget, XtPointer, XtPointer);
void Button1DownUpAction(Widget, XEvent *, String *, Cardinal *);
void Button1DownAction(Widget, XEvent *, String *, Cardinal *);
void Button1MotionAction(Widget, XEvent *, String *, Cardinal *);
void Button1UpAction(Widget, XEvent *, String *, Cardinal *);
void Button2DownUpAction(Widget, XEvent *, String *, Cardinal *);
void Button2DownAction(Widget, XEvent *, String *, Cardinal *);
void Button2MotionAction(Widget, XEvent *, String *, Cardinal *);
void Button2UpAction(Widget, XEvent *, String *, Cardinal *);
void dw_draw(Widget w, XtPointer data, XtPointer callData);
void dw_resize(Widget w, XtPointer data, XtPointer callData);
void dw_init(Widget w, XtPointer data, XtPointer callData);
//----------------------------------------------------------------------------
// Creator for X11 version of drawing window.
xvt_drawwin::xvt_drawwin(const char * filename, xvt_mainwin & mw)
  : vt_drawwin(filename,mw), xmvt(mw)
{
  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Create an application shell for the draw window
  draw_shell = XtVaAppCreateShell("xvistool","drawshell",
				  topLevelShellWidgetClass,
				  xmvt.display,
				  XmNtitle,filename,
				  NULL);

  //--------------------------------------------------------------------------
  // Make sure the window is properly deleted if the window manager kills
  // the window instead of the user using the file menu close option.
  Atom WM_DELETE_WINDOW = XmInternAtom(xmvt.display, "WM_DELETE_WINDOW",
				       False);
  XmAddWMProtocolCallback(draw_shell, WM_DELETE_WINDOW, dw_file_close, this);

  //--------------------------------------------------------------------------
  // Add these actions.
  static XtActionsRec dw_actionsTable[] = {
    {"Button1DownAction",Button1DownAction},
    {"Button1MotionAction",Button1MotionAction},
    {"Button1UpAction",Button1UpAction},
    {"Button2DownAction",Button2DownAction},
    {"Button2MotionAction",Button2MotionAction},
    {"Button2UpAction",Button2UpAction}
  };
  XtAppAddActions(xmvt.app, dw_actionsTable, XtNumber(dw_actionsTable));

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // main window contains MenuBar
  main_w = XtVaCreateManagedWidget("drawwin",xmMainWindowWidgetClass,
				   draw_shell,
				   XmNuserData,this,
				   NULL);

  XtAddEventHandler(draw_shell, StructureNotifyMask, False,
		    mapStateChanged, this);

  //--------------------------------------------------------------------------
  // Create the MenuBar
  Arg args[3];
  XtSetArg(args[0], XmNuserData, this);
  menu_bar = XmCreateMenuBar(main_w, "menubar", args, 1);
  XtManageChild(menu_bar);
  int OLn=0;
  Arg OLargs[10];
  XtSetArg(OLargs[0], XmNuserData, this); OLn++;
  if(xmvt.overlayVisual) {
    XtSetArg(OLargs[OLn], XmNvisual, xmvt.overlayVisual); OLn++;
    XtSetArg(OLargs[OLn], XmNdepth, xmvt.overlayDepth); OLn++;
    XtSetArg(OLargs[OLn], XmNcolormap, xmvt.overlayColormap); OLn++;
  }

  //--------------------------------------------------------------------------
  // First menu is the File menu
  Widget menu_pane = XmCreatePulldownMenu(menu_bar, "menupane", OLargs, OLn);
  if(xmvt.overlayVisual)
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) dw_ensurePulldownColormapInstalled, this);
  Widget btn;
  btn = XmCreatePushButtonGadget(menu_pane, "Open...", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) dw_file_open, this);
  XtManageChild(btn);
  btn = XmCreateSeparatorGadget(menu_pane, NULL, args, 1);
  XtManageChild(btn);
  btn = XmCreatePushButtonGadget(menu_pane, "Close", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) dw_file_close,this);
  XtManageChild(btn);
  XtSetArg(args[1], XmNsubMenuId, menu_pane);
  Widget cascade = XmCreateCascadeButton(menu_bar, "File", args, 2);
  XtManageChild(cascade);

  //--------------------------------------------------------------------------
  // Last menu is the Help menu
  menu_pane = XmCreatePulldownMenu(menu_bar, "menupane", OLargs, OLn);
  if(xmvt.overlayVisual)
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) dw_ensurePulldownColormapInstalled, this);
  btn = XmCreatePushButtonGadget(menu_pane, "Help", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) dw_help, this);
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
  popup = XmCreatePopupMenu(frame, "menupane", OLargs, OLn);
  if(xmvt.overlayVisual)
    XtAddCallback(XtParent(popup), XmNpopupCallback,
		  (XtCallbackProc) dw_ensurePulldownColormapInstalled, this);
  btn = XmCreatePushButtonGadget(popup, "Reset", args, 1);
  XtAddCallback(btn, XmNactivateCallback, (XtCallbackProc) dw_popup_reset,
		this);
  XtManageChild(btn);
  btn = XmCreateSeparatorGadget(popup, NULL, args, 1);
  XtManageChild(btn);
  XtSetArg(args[1], XmNindicatorType, XmN_OF_MANY);
  XtSetArg(args[2], XmNvisibleWhenOff, True); // needed for lesstif
  btn = XmCreateToggleButtonGadget(popup, "Animate", args, 3);
  XtAddCallback(btn, XmNvalueChangedCallback,
		(XtCallbackProc) dw_popup_animate, this);
  XmToggleButtonSetState(btn, animate, False);
  XtManageChild(btn);
  XtAddEventHandler(frame, ButtonPressMask, False, activatePopup, popup);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // OpenGL drawing area
  glx_area = XtVaCreateManagedWidget("glxarea",glwMDrawingAreaWidgetClass,
				     frame,
				     GLwNvisualInfo,xmvt.vi,
				     XmNuserData,this,
				     NULL);
  XtVaGetValues(glx_area, XtNwidth, &viewWidth, XtNheight, &viewHeight, NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  XtAddCallback(glx_area, XmNexposeCallback, dw_draw, NULL);
  XtAddCallback(glx_area, XmNresizeCallback, dw_resize, NULL);
  XtAddCallback(glx_area, GLwNginitCallback, dw_init, NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Set up mouse button actions
  static char * glxareaTranslations =
    "#override\n\
    <Btn1Down>:Button1DownAction()\n\
    <Btn1Motion>:Button1MotionAction()\n\
    <Btn1Up>:Button1UpAction()\n\
    <Btn2Down>:Button2DownAction()\n\
    <Btn2Motion>:Button2MotionAction()\n\
    <Btn2Up>:Button2UpAction()\n";
  XtTranslations trans = XtParseTranslationTable(glxareaTranslations);
  XtOverrideTranslations(glx_area, trans);

  //--------------------------------------------------------------------------
  // Create OpenGL rendering context with no display list shareing and
  // with direct rendering favored
  cx = glXCreateContext(xmvt.display,xmvt.vi,None,TRUE);
  if(!cx) XtAppError(xmvt.app,"could not create rendering context");

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
  // Make sure the animation TimeOut routine is handled correctly when
  // a window is deleted.
  if(animate) { 
    if((xmvt.animateCount == 1) && visible) XtRemoveTimeOut(xmvt.animateID);
    xmvt.animateCount--;
    if(visible) {
      if(xmvt.animateCount ==  xmvt.animateHiddenCount)
	XtRemoveTimeOut(xmvt.animateID);
    } else xmvt.animateHiddenCount--;
  }

  // *** NOTE *** We are destroying a Shell and not a Widget ????
  // *** Mesa error message ***
  //     "Warning: XtRemoveGrab asked to remove a widget not on the list"
  // cout << "in xvt_drawwin destructor" << endl;
  XtDestroyWidget(draw_shell);
}

void xvt_mainwin::redisplay()
{
  typedef list<vt_drawwin *>::iterator I_dw;
  I_dw p;
  for(p = draw_list.begin(); p != draw_list.end(); p++) {
    xvt_drawwin & dw = *((xvt_drawwin *) *p);
    if(dw.redisplay) {
      dw.draw();
      dw.redisplay = false;
    }
  }
  redisplayPending = false;
}

void xvt_drawwin::init(int width, int height)
{
  glx_win = (GLXDrawable) XtWindow(glx_area);
  glXMakeCurrent(xmvt.display,glx_win,cx);
  vt_drawwin::init(width, height);
}

void xvt_drawwin::draw()
{
  if(!visible) return;
  glXMakeCurrent(xmvt.display,glx_win,cx);
  vt_drawwin::draw();
  glXSwapBuffers(xmvt.display,glx_win);
}

void xvt_drawwin::resize(int new_width, int new_height)
{
  glXMakeCurrent(xmvt.display,glx_win,cx);
  vt_drawwin::resize(new_width, new_height);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void handleRedisplay(xvt_mainwin * xmvt);
//----------------------------------------------------------------------------
void xvt_drawwin::postRedisplay()
{
  redisplay = true;
  if(!xmvt.redisplayPending) {
    xmvt.redisplayID = XtAppAddWorkProc(xmvt.app,
					(XtWorkProc) handleRedisplay, &xmvt);
    xmvt.redisplayPending = true;
  }
}
