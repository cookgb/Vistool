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
#include <Xm/ToggleB.h>
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
void ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
void mw_file_open_GIO(Widget, XtPointer, XtPointer);
void mw_file_open_1DDump(Widget, XtPointer, XtPointer);
void mw_file_open_1DDAb(Widget, XtPointer, XtPointer);
void mw_file_quit(Widget, XtPointer, XtPointer);
void mw_help(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Creator for X11 version of main window.
xvt_mainwin::xvt_mainwin(int & argc, char ** argv)
  : vt_mainwin(), Open_Dialog(0), vi(0), overlayDepth(0), doubleBuffer(true),
    animateCount(0), animateHiddenCount(0),animateID(0),redisplayPending(0)
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
  
  //--------------------------------------------------------------------------
  // Initialize toolkit and parse command line options.
  top_shell = XtVaAppInitialize(&app,"xvistool",NULL,0,&argc,argv,
				fallbackResources,NULL,NULL);

  //--------------------------------------------------------------------------
  // Get the X11 display
  display = XtDisplay(top_shell);

  //--------------------------------------------------------------------------
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
				   NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Create the MenuBar
  XmString m_file = XmStringCreateLocalized("File");
  XmString m_help = XmStringCreateLocalized("Help");
  menu_bar = XmVaCreateSimpleMenuBar(main_w,"menubar",
				     XmVaCASCADEBUTTON, m_file, 'F',
				     XmVaCASCADEBUTTON, m_help, 'H',
				     NULL);
  XmStringFree(m_file);

  // Tell the MenuBar which button is th help menu
  if(Widget w = XtNameToWidget(menu_bar,"button_1"))
    XtVaSetValues(menu_bar,XmNmenuHelpWidget,w,NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // First menu is the File menu
  XmString m_open = XmStringCreateLocalized("Open...");
  XmString m_quit = XmStringCreateLocalized("Quit");
  //
#define FILEMENU XmVaCASCADEBUTTON,m_open,'n',\
                 XmVaSEPARATOR,\
	         XmVaPUSHBUTTON,m_quit,'Q',NULL,NULL
  //
  Widget menu_pane;
  if(overlayVisual) {
    menu_pane =
      XmVaCreateSimplePulldownMenu(menu_bar,"filemenu",0,NULL,
				   FILEMENU,
				   XmNvisual, overlayVisual,
				   XmNdepth, overlayDepth,
				   XmNcolormap, overlayColormap,
				   NULL);
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, this);
  } else {
    menu_pane =
      XmVaCreateSimplePulldownMenu(menu_bar,"filemenu",0,NULL,
				   FILEMENU,
				   NULL);
  }
#undef FILEMENU
  XmStringFree(m_open);
  XmStringFree(m_quit);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(menu_pane,"button_1"))
    XtAddCallback(w, XmNactivateCallback, mw_file_quit, this);

  //--------------------------------------------------------------------------
  // Open submenu
  XmString m_gio_format = XmStringCreateLocalized("GIO Format");
  XmString m_1ddump_format = XmStringCreateLocalized("1DDump Format");
  XmString m_1ddump_abscissa = XmStringCreateLocalized("1DDump Abscissa");
  //
#define OPENMENU XmVaPUSHBUTTON,m_gio_format,'G',NULL,NULL,\
                 XmVaSEPARATOR,\
	         XmVaPUSHBUTTON,m_1ddump_format,'1',NULL,NULL,\
		 XmVaCHECKBUTTON,m_1ddump_abscissa,NULL,NULL,NULL
  //
  Widget open_popout;
  if(overlayVisual) {
    open_popout =
      XmVaCreateSimplePulldownMenu(menu_pane, "options", 0, NULL,
				   OPENMENU,
				   XmNvisual, overlayVisual,
				   XmNdepth, overlayDepth,
				   XmNcolormap, overlayColormap,
				   NULL);
    XtAddCallback(XtParent(open_popout), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, this);
  } else {
    open_popout =
      XmVaCreateSimplePulldownMenu(menu_pane, "options", 0, NULL,
				   OPENMENU,
				   NULL);
  }
#undef OPENMENU
  XmStringFree(m_gio_format);
  XmStringFree(m_1ddump_format);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(open_popout,"button_0"))
    XtAddCallback(w, XmNactivateCallback, mw_file_open_GIO, this);
  if(Widget w = XtNameToWidget(open_popout,"button_1"))
    XtAddCallback(w, XmNactivateCallback, mw_file_open_1DDump, this);
  if(Widget w = XtNameToWidget(open_popout,"button_2")) {
    CheckButton_1DAbs = w;
    XtAddCallback(w, XmNvalueChangedCallback, mw_file_open_1DDAb, this);
    XmToggleButtonSetState(w, Abscissa_Set, False);
  }

  //--------------------------------------------------------------------------
  // Last menu is the Help menu
  //
#define HELPMENU XmVaPUSHBUTTON,m_help,'H',NULL,NULL
  //
  if(overlayVisual) {
    menu_pane =
      XmVaCreateSimplePulldownMenu(menu_bar,"helpmenu",1,NULL,
				   HELPMENU,
				   XmNvisual, overlayVisual,
				   XmNdepth, overlayDepth,
				   XmNcolormap, overlayColormap,
				   NULL);
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, this);
  } else {
    menu_pane =
      XmVaCreateSimplePulldownMenu(menu_bar,"helpmenu",1,NULL,
				   HELPMENU,
				   NULL);
  }
#undef HELPMENU
  XmStringFree(m_help);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(menu_pane,"button_0"))
    XtAddCallback(w, XmNactivateCallback, mw_help, this);

  XtManageChild(menu_bar);

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
void Button1DownUpAction(Widget, XEvent *, String *, Cardinal *);
void Button1DownAction(Widget, XEvent *, String *, Cardinal *);
void Button1MotionAction(Widget, XEvent *, String *, Cardinal *);
void Button1UpAction(Widget, XEvent *, String *, Cardinal *);
void Button2DownUpAction(Widget, XEvent *, String *, Cardinal *);
void Button2DownAction(Widget, XEvent *, String *, Cardinal *);
void Button2MotionAction(Widget, XEvent *, String *, Cardinal *);
void Button2UpAction(Widget, XEvent *, String *, Cardinal *);
void mapStateChanged(Widget, XtPointer, XEvent *, Boolean *);
void dw_animate(Widget, XtPointer, XtPointer);
void dw_reset_animate(Widget, XtPointer, XtPointer);
void ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
void dw_file_open(Widget, XtPointer, XtPointer);
void dw_file_open_GIO(Widget, XtPointer, XtPointer);
void dw_file_open_1DDump(Widget, XtPointer, XtPointer);
void dw_file_open_1DDAb(Widget, XtPointer, XtPointer);
void dw_file_close(Widget, XtPointer, XtPointer);
void dw_help(Widget, XtPointer, XtPointer);
void activatePopup(Widget, XtPointer, XEvent *, Boolean *);
void dw_popup_reset(Widget, XtPointer, XtPointer);
void dw_draw(Widget, XtPointer, XtPointer);
void dw_resize(Widget, XtPointer, XtPointer);
void dw_init(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
void apply_log(Widget, XtPointer, XtPointer);
void apply_ln(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Creator for X11 version of drawing window.
xvt_drawwin::xvt_drawwin(const char * filename, xvt_mainwin & mw,
			 XmString & dir, XmString & pattern)
  : vt_drawwin(filename,mw), xmvt(mw), Open_Dialog(0)
{
  // Save the directory so that the new Open will start searching here.
  search_dir = XmStringCopy(dir);
  search_pattern = XmStringCopy(pattern);

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
				   NULL);

  XtAddEventHandler(draw_shell, StructureNotifyMask, False, mapStateChanged,
		    this);

  //--------------------------------------------------------------------------
  // Create the MenuBar
  XmString m_file = XmStringCreateLocalized("File");
  XmString m_help = XmStringCreateLocalized("Help");
  menu_bar = XmVaCreateSimpleMenuBar(main_w,"menubar",
				     XmVaCASCADEBUTTON, m_file, 'F',
				     XmVaCASCADEBUTTON, m_help, 'H',
				     NULL);
  XmStringFree(m_file);

  // Tell the MenuBar which button is th help menu
  if(Widget w = XtNameToWidget(menu_bar,"button_1"))
    XtVaSetValues(menu_bar,XmNmenuHelpWidget,w,NULL);

  //--------------------------------------------------------------------------
  // First menu is the File menu
  XmString m_open  = XmStringCreateLocalized("Open...");
  XmString m_close = XmStringCreateLocalized("Close");
  //
#define FILEMENU XmVaCASCADEBUTTON,m_open,'n',\
                 XmVaSEPARATOR,\
	         XmVaPUSHBUTTON,m_close,'C',NULL,NULL
  //
  Widget menu_pane;
  if(xmvt.overlayVisual) {
    menu_pane =
      XmVaCreateSimplePulldownMenu(menu_bar,"filemenu",0,NULL,
				   FILEMENU,
				   XmNvisual, xmvt.overlayVisual,
				   XmNdepth, xmvt.overlayDepth,
				   XmNcolormap, xmvt.overlayColormap,
				   NULL);
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, &xmvt);
  } else {
    menu_pane =
      XmVaCreateSimplePulldownMenu(menu_bar,"filemenu",0,NULL,
				   FILEMENU,
				   NULL);
  }
#undef FILEMENU
  XmStringFree(m_open);
  XmStringFree(m_close);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(menu_pane,"button_1"))
    XtAddCallback(w, XmNactivateCallback, dw_file_close, this);

  //--------------------------------------------------------------------------
  // Open submenu
  XmString m_gio_format = XmStringCreateLocalized("GIO Format");
  XmString m_1ddump_format = XmStringCreateLocalized("1DDump format");
  XmString m_1ddump_abscissa = XmStringCreateLocalized("1DDump Abscissa");
  //
#define OPENMENU XmVaPUSHBUTTON,m_gio_format,'G',NULL,NULL,\
                 XmVaSEPARATOR,\
	         XmVaPUSHBUTTON,m_1ddump_format,'1',NULL,NULL,\
		 XmVaCHECKBUTTON,m_1ddump_abscissa,NULL,NULL,NULL
  //
  Widget open_popout;
  if(xmvt.overlayVisual) {
    open_popout =
      XmVaCreateSimplePulldownMenu(menu_pane, "options", 0, NULL,
				   OPENMENU,
				   XmNvisual, xmvt.overlayVisual,
				   XmNdepth, xmvt.overlayDepth,
				   XmNcolormap, xmvt.overlayColormap,
				   NULL);
    XtAddCallback(XtParent(open_popout), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, &xmvt);
  } else {
    open_popout =
      XmVaCreateSimplePulldownMenu(menu_pane, "options", 0, NULL,
				   OPENMENU,
				   NULL);
  }
#undef OPENMENU
  XmStringFree(m_gio_format);
  XmStringFree(m_1ddump_format);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(open_popout,"button_0"))
    XtAddCallback(w, XmNactivateCallback,dw_file_open_GIO, this);
  if(Widget w = XtNameToWidget(open_popout,"button_1"))
    XtAddCallback(w, XmNactivateCallback,dw_file_open_1DDump, this);
  if(Widget w = XtNameToWidget(open_popout,"button_2")) {
    CheckButton_1DAbs = w;
    XtAddCallback(w, XmNvalueChangedCallback,dw_file_open_1DDAb, this);
    XmToggleButtonSetState(w, Abscissa_Set, False);
  }

  //--------------------------------------------------------------------------
  // Last menu is the Help menu
  //
#define HELPMENU XmVaPUSHBUTTON,m_help,'H',NULL,NULL
  //
  if(xmvt.overlayVisual) {
    menu_pane =
      XmVaCreateSimplePulldownMenu(menu_bar,"helpmenu",1,NULL,
				   HELPMENU,
				   XmNvisual,  xmvt.overlayVisual,
				   XmNdepth,  xmvt.overlayDepth,
				   XmNcolormap,  xmvt.overlayColormap,
				   NULL);
    XtAddCallback(XtParent(menu_pane), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, &xmvt);
  } else {
    menu_pane =
      XmVaCreateSimplePulldownMenu(menu_bar,"helpmenu",1,NULL,
				   HELPMENU,
				   NULL);
  }
#undef HELPMENU
  XmStringFree(m_help);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(menu_pane,"button_0"))
    XtAddCallback(w, XmNactivateCallback, dw_help, this);

  XtManageChild(menu_bar);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Frame area to contain OpenGL drawing area
  frame = XtVaCreateManagedWidget("frame",xmFrameWidgetClass,
				  main_w,
				  NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Create Popup menu for frame area of drawing window
  XmString m_reset = XmStringCreateLocalized("Reset");
  XmString m_apply = XmStringCreateLocalized("Apply");
  XmString m_anim = XmStringCreateLocalized("Animate");
  //
#define POPUPMENU XmVaPUSHBUTTON,m_reset,NULL,NULL,NULL,\
                  XmVaCASCADEBUTTON,m_apply,NULL,\
                  XmVaSEPARATOR,\
		  XmVaCASCADEBUTTON,m_anim,NULL
#ifdef LESSTIF_VERSION
  #define PUB_APPLY 1
  #define PUB_ANIMATE 2
#else
  #define PUB_APPLY 1
  #define PUB_ANIMATE 3
#endif
  //
  if(xmvt.overlayVisual) {
    popup =
      XmVaCreateSimplePopupMenu(frame,"popupmenu",NULL,
				POPUPMENU,
				XmNvisual, xmvt.overlayVisual,
				XmNdepth, xmvt.overlayDepth,
				XmNcolormap, xmvt.overlayColormap,
				NULL);
    XtAddCallback(XtParent(popup), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, &xmvt);
  } else {
    popup =
      XmVaCreateSimplePopupMenu(frame,"popupmenu",NULL,
				POPUPMENU,
				NULL);
  }
#undef POPUPMENU
//    XmStringFree(m_reset);
  XmStringFree(m_apply);
//    XmStringFree(m_anim);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(popup,"button_0"))
    XtAddCallback(w, XmNactivateCallback, dw_popup_reset, this);
//    if(Widget w = XtNameToWidget(popup,"button_2")) {
//      XtAddCallback(w, XmNvalueChangedCallback, dw_popup_animate, this);
//      XmToggleButtonSetState(w, animate, False);
//    }

  XtAddEventHandler(frame, ButtonPressMask, False, activatePopup, popup);

  //--------------------------------------------------------------------------
  // Apply submenu
  XmString m_f_log = XmStringCreateLocalized("log");
  XmString m_f_ln = XmStringCreateLocalized("ln");
  //
#define APPLYMENU XmVaPUSHBUTTON,m_f_log,NULL,NULL,NULL,\
	          XmVaPUSHBUTTON,m_f_ln,NULL,NULL,NULL
  //
  Widget apply_popout;
  if(xmvt.overlayVisual) {
    apply_popout =
      XmVaCreateSimplePulldownMenu(popup, "apply", PUB_APPLY, NULL,
				   APPLYMENU,
				   XmNvisual, xmvt.overlayVisual,
				   XmNdepth, xmvt.overlayDepth,
				   XmNcolormap, xmvt.overlayColormap,
				   NULL);
    XtAddCallback(XtParent(apply_popout), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, &xmvt);
  } else {
    apply_popout =
      XmVaCreateSimplePulldownMenu(popup, "apply", PUB_APPLY, NULL,
				   APPLYMENU,
				   NULL);
  }
#undef APPLYMENU
  XmStringFree(m_f_log);
  XmStringFree(m_f_ln);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(apply_popout,"button_0"))
    XtAddCallback(w, XmNactivateCallback, apply_log, this);
  if(Widget w = XtNameToWidget(apply_popout,"button_1"))
    XtAddCallback(w, XmNactivateCallback, apply_log, this);

  //--------------------------------------------------------------------------
  // Animate submenu
//    XmString m_anim = XmStringCreateLocalized("Animate");
//    XmString m_reset = XmStringCreateLocalized("Reset");
  //
#define ANIMATEMENU XmVaCHECKBUTTON,m_anim,NULL,NULL,NULL,\
                    XmVaPUSHBUTTON,m_reset,NULL,NULL,NULL
  //
  Widget animate_popout;
  if(xmvt.overlayVisual) {
    animate_popout =
      XmVaCreateSimplePulldownMenu(popup, "animate", PUB_ANIMATE, NULL,
				   ANIMATEMENU,
				   XmNvisual, xmvt.overlayVisual,
				   XmNdepth, xmvt.overlayDepth,
				   XmNcolormap, xmvt.overlayColormap,
				   NULL);
    XtAddCallback(XtParent(animate_popout), XmNpopupCallback,
		  (XtCallbackProc) ensurePulldownColormapInstalled, &xmvt);
  } else {
    animate_popout =
      XmVaCreateSimplePulldownMenu(popup, "animate", PUB_ANIMATE, NULL,
				   ANIMATEMENU,
				   NULL);
  }
#undef APPLYMENU
  XmStringFree(m_anim);
  XmStringFree(m_reset);
  // Set the callback routines for each menu button.
  if(Widget w = XtNameToWidget(animate_popout,"button_0")) {
    XtAddCallback(w, XmNvalueChangedCallback, dw_animate, this);
    XmToggleButtonSetState(w, animate, False);
  }
  if(Widget w = XtNameToWidget(animate_popout,"button_1"))
    XtAddCallback(w, XmNactivateCallback, dw_reset_animate, this);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // OpenGL drawing area
  glx_area = XtVaCreateManagedWidget("glxarea",glwMDrawingAreaWidgetClass,
				     frame,
				     GLwNvisualInfo,xmvt.vi,
				     XmNuserData,this, // For Button actions
				     NULL);
  XtVaGetValues(glx_area, XtNwidth, &viewWidth, XtNheight, &viewHeight, NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  XtAddCallback(glx_area, XmNexposeCallback, dw_draw, this);
  XtAddCallback(glx_area, XmNresizeCallback, dw_resize, this);
  XtAddCallback(glx_area, GLwNginitCallback, dw_init, this);

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
  glXMakeCurrent(xmvt.Xdisplay(),glx_win,cx);
  vt_drawwin::draw();
  glXSwapBuffers(xmvt.Xdisplay(),glx_win);
}

void xvt_drawwin::resize(int new_width, int new_height)
{
  glXMakeCurrent(xmvt.Xdisplay(),glx_win,cx);
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
    xmvt.redisplayID = XtAppAddWorkProc(xmvt.Xapp(),
					(XtWorkProc) handleRedisplay, &xmvt);
    xmvt.redisplayPending = true;
  }
}
