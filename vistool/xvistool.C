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
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>
#include <Xm/Frame.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>

#include <GL/GLwMDrawA.h>
#include <GL/glx.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

#include "xvistool.h"
#include "xMenu.h"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Create main window and start event loop;
int main(int argc, char * argv[])
{
  xvt_mainwin * xvt = new xvt_mainwin(argc,argv);
  xvt->Loop();
  return 1;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
void mw_file_open_GIO(Widget, XtPointer, XtPointer);
void mw_file_open_1DDump(Widget, XtPointer, XtPointer);
void mw_file_open_1DDAb(Widget, XtPointer, XtPointer);
void mw_file_quit(Widget, XtPointer, XtPointer);
void mw_windowlist(Widget, XtPointer, XtPointer);
void mw_datasetlist(Widget, XtPointer, XtPointer);
void mw_anim_sync(Widget, XtPointer, XtPointer);
void mw_anim_unsync(Widget, XtPointer, XtPointer);
void mw_help(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Creator for X11 version of main window.
xvt_mainwin::xvt_mainwin(int & argc, char ** argv)
  : vt_mainwin(), Open_Dialog(0), vi(0), overlayVisual(0), overlayDepth(0),
    doubleBuffer(true),
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
    "*XmScrolledWindow.width: 170",
    "*XmScrolledWindow.scrollBarDisplayPolicy: as_needed",
    "*XmList.listSizePolicy: constant",
    "*windowlist.width: 170",
    "*windowlist.visibleItemCount: 4",
    "*windowlist.selectionPolicy: extended_select",
    "*datasetlist.width: 170",
    "*datasetlist.visibleItemCount: 4",
    "*datasetlist.selectionPolicy: extended_select",
    "*infoframe.width: 450",
    "*infoframe.height: 80",
//      "*main.width: 350",
//      "*main.height: 150",
    "*glxarea.width: 300",
    "*glxarea.height: 300",
    "*infolabel.fontList: -*-courier-medium-r-*--*-120-*=TAG1,\
                          -*-courier-medium-r-*--*-140-*=TAG2,\
                          -*-courier-medium-r-*--*-180-*=TAG3",
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
  menu_bar = XmCreateMenuBar(main_w, "menubar", NULL, 0);

  // Set OverlayVisual structure for menus
  MenuOverlayVisual * overlay_Visual = 0;
  if(overlayVisual) {
    overlay_Visual = new MenuOverlayVisual;
    overlay_Visual->overlayVisual   = overlayVisual;
    overlay_Visual->overlayDepth    = overlayDepth;
    overlay_Visual->overlayColormap = overlayColormap;
    overlay_Visual->popupCallback   =
      (XtCallbackProc) ensurePulldownColormapInstalled;
    overlay_Visual->callback_data   = this;
  }

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // First menu is the File menu
  MenuItem Open_menu[] = {
    { "GIO Format", &xmPushButtonGadgetClass, 'G', NULL, NULL, 0,
      mw_file_open_GIO, this, NULL},
    { "_sep1", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "1DDump Format", &xmPushButtonGadgetClass, '1', NULL, NULL, 0,
      mw_file_open_1DDump, this, NULL},
    { "1DDump Abscissa", &xmToggleButtonGadgetClass, '\0', NULL, NULL,
      Abscissa_Set, mw_file_open_1DDAb, this, NULL},
    NULL,
  };
  MenuItem File_menu[] = {
    { "Open...", &xmCascadeButtonGadgetClass, 'n', NULL, NULL, 0,
      NULL, this, Open_menu},
    { "_sep1", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "Quit", &xmPushButtonGadgetClass, 'Q', "Ctrl<Key>C", "Ctrl+C", 0,
      mw_file_quit, this, NULL},
    NULL,
  };
  BuildMenu(menu_bar, XmMENU_PULLDOWN, "File", 'F', overlay_Visual,
	    false, File_menu);

  //--------------------------------------------------------------------------
  // Next menu is the Animate menu
  //
  MenuItem Animate_menu[] = {
    { "Synchronize", &xmPushButtonGadgetClass, 'S', NULL, NULL,0,
      mw_anim_sync, this, NULL},
    { "Un-Sync", &xmPushButtonGadgetClass, 'U', NULL, NULL,0,
      mw_anim_unsync, this, NULL},
    NULL,
  };
  Widget Animate_m =
    BuildMenu(menu_bar, XmMENU_PULLDOWN, "Animate", 'A', overlay_Visual,
	      false, Animate_menu);

  //--------------------------------------------------------------------------
  // Last menu is the Help menu
  //
  MenuItem Help_menu[] = {
    { "Help", &xmPushButtonGadgetClass, 'H', NULL, NULL,0,
      mw_help, this, NULL},
    NULL,
  };
  Widget Help_m =
    BuildMenu(menu_bar, XmMENU_PULLDOWN, "Help", 'H', overlay_Visual,
	      false, Help_menu);
  XtVaSetValues(menu_bar, XmNmenuHelpWidget, Help_m, NULL);

  XtManageChild(menu_bar);
  delete overlay_Visual;

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  Widget mw_form = XtVaCreateManagedWidget("form", xmFormWidgetClass,
					   main_w, NULL);

  Arg args[10];
  int nl = 0;

  Widget infoframe = XtVaCreateManagedWidget("infoframe",xmFrameWidgetClass,
					     mw_form,
					     XmNleftAttachment, XmATTACH_FORM,
					     XmNbottomAttachment,XmATTACH_FORM,
					     XmNrightAttachment, XmATTACH_FORM,
					     XmNleftOffset, 4,
					     XmNbottomOffset, 4,
					     XmNrightOffset, 4,
					     NULL);
  info_label= XtVaCreateManagedWidget("infolabel",xmLabelGadgetClass,
				      infoframe,
				      XmNalignment,XmALIGNMENT_BEGINNING,
				      NULL);
  NULL_String = XmStringCreateLocalized("");
  XtVaSetValues(info_label, XmNlabelString, NULL_String, NULL);
					     
  Widget inflab = XtVaCreateManagedWidget("Info:",xmLabelGadgetClass,
					  mw_form,
					  XmNbottomAttachment, XmATTACH_WIDGET,
					  XmNbottomWidget, infoframe,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNbottomOffset, 4,
					  XmNleftOffset, 4,
					  NULL);

  Widget winlab = XtVaCreateManagedWidget("Windows:",xmLabelGadgetClass,
					  mw_form,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNtopOffset, 4,
					  XmNleftOffset, 4,
					  NULL);
  nl = 0;
  XtSetArg(args[nl],XmNscrollingPolicy,XmAUTOMATIC); nl++;
  XtSetArg(args[nl],XmNtopAttachment,XmATTACH_WIDGET); nl++;
  XtSetArg(args[nl],XmNtopWidget,winlab); nl++;
  XtSetArg(args[nl],XmNleftAttachment,XmATTACH_OPPOSITE_WIDGET); nl++;
  XtSetArg(args[nl],XmNleftWidget,winlab); nl++;
  XtSetArg(args[nl],XmNbottomAttachment,XmATTACH_WIDGET); nl++;
  XtSetArg(args[nl],XmNbottomWidget,inflab); nl++;
  XtSetArg(args[nl],XmNbottomOffset,4); nl++;
  XtSetArg(args[nl],XmNtopOffset,2); nl++;
  window_list = XmCreateScrolledList(mw_form,"windowlist",args,nl);
  Widget listpar = XtParent(window_list);

  Widget dslab = XtVaCreateManagedWidget("Data sets:",xmLabelGadgetClass,
					 mw_form,
  					 XmNleftAttachment, XmATTACH_WIDGET,
  					 XmNleftWidget, listpar,
					 XmNleftOffset,8,
					 XmNbottomAttachment,
					 XmATTACH_OPPOSITE_WIDGET,
					 XmNbottomWidget, winlab,
					 NULL);
  nl = 0;
  XtSetArg(args[nl],XmNscrollingPolicy,XmAUTOMATIC); nl++;
  XtSetArg(args[nl],XmNtopAttachment,XmATTACH_OPPOSITE_WIDGET); nl++;
  XtSetArg(args[nl],XmNtopWidget,listpar); nl++;
  XtSetArg(args[nl],XmNleftAttachment,XmATTACH_OPPOSITE_WIDGET); nl++;
  XtSetArg(args[nl],XmNleftWidget,dslab); nl++;
  XtSetArg(args[nl],XmNbottomAttachment,XmATTACH_WIDGET); nl++;
  XtSetArg(args[nl],XmNbottomWidget,inflab); nl++;
  XtSetArg(args[nl],XmNrightAttachment,XmATTACH_FORM); nl++;
  XtSetArg(args[nl],XmNrightOffset,4); nl++;
  XtSetArg(args[nl],XmNbottomOffset,4); nl++;
  dataset_list = XmCreateScrolledList(mw_form,"datasetlist",args,nl);

  XtManageChild(window_list);
  XtManageChild(dataset_list);

  XtAddCallback(window_list, XmNextendedSelectionCallback,
		mw_windowlist, this);
  XtAddCallback(dataset_list, XmNextendedSelectionCallback,
		mw_datasetlist, this);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  XmMainWindowSetAreas(main_w,menu_bar,NULL,NULL,NULL,mw_form);

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
  if(Open_Dialog) XtDestroyWidget(Open_Dialog);
  XtDestroyWidget(top_shell);
  dw_listed = 0;
  ds_listed = 0;
  XmStringFree(NULL_String);
}

void xvt_mainwin::redisplay()
{
  typedef std::list<vt_drawwin *>::iterator I_dw;
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

void xvt_mainwin::RegisterDW(const char *const window)
{
  XmString WinName = XmStringCreateLocalized((String)window);
  XmListAddItemUnselected(window_list,WinName,0);
  XmStringFree(WinName);
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Creator for X11 version of drawing window.
xvt_drawwin::xvt_drawwin(const char *const filename, xvt_mainwin & mw,
			 const XmString dir, const XmString pattern)
  : vt_drawwin(filename,mw), xmvt(mw), Open_Dialog(0), swapcount(0),
    draw_RB(false)
{
  // Save the directory so that the new Open will start searching here.
  search_dir = XmStringCopy(dir);
  search_pattern = XmStringCopy(pattern);

  CreateWindow();
}

xvt_drawwin::xvt_drawwin(const xvt_drawwin & xdw)
  : vt_drawwin(xdw), xmvt(xdw.xmvt), Open_Dialog(0), swapcount(0),
    draw_RB(false)
{
  // Save the directory so that the new Open will start searching here.
  if(!xdw.OpenDialog()) {
    search_dir = XmStringCopy(xdw.search_dir);
    search_pattern = XmStringCopy(xdw.search_pattern);
  } else {
    XtVaGetValues(xdw.OpenDialog(),XmNdirectory,&search_dir,
		  XmNpattern,&search_pattern,NULL);
  }

  CreateWindow();
  
  std::list<vt_data_series *>::const_iterator p;
  for(p = xdw.data_list.begin(); p != xdw.data_list.end(); p++)
    Add(new vt_data_series(**p));
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
void dw_LockFullFrame(Widget, XtPointer, XtPointer);
void dw_ZoomCurrent(Widget, XtPointer, XtPointer);
void dw_ZoomReset(Widget, XtPointer, XtPointer);
void dw_UnZoom(Widget, XtPointer, XtPointer);
void dw_animate(Widget, XtPointer, XtPointer);
void dw_stepforward(Widget, XtPointer, XtPointer);
void dw_stepbackward(Widget, XtPointer, XtPointer);
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
void apply_copy(Widget, XtPointer, XtPointer);
void apply_abs(Widget, XtPointer, XtPointer);
void apply_log(Widget, XtPointer, XtPointer);
void apply_ln(Widget, XtPointer, XtPointer);
void apply_dt(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
void norm_Linf(Widget, XtPointer, XtPointer);
void norm_L1(Widget, XtPointer, XtPointer);
void norm_L2(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Code to create X window
void xvt_drawwin::CreateWindow()
{
  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Create an application shell for the draw window
  draw_shell = XtVaAppCreateShell("xvistool","drawshell",
				  topLevelShellWidgetClass,
				  xmvt.display,
				  XmNtitle,name,
				  NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Register the window in the main window list
  xmvt.RegisterDW(name);

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
				   XmNcommandWindowLocation,
				   XmCOMMAND_BELOW_WORKSPACE,
				   NULL);

  XtAddEventHandler(draw_shell, StructureNotifyMask, False, mapStateChanged,
		    this);

  //--------------------------------------------------------------------------
  // Create the MenuBar
  menu_bar = XmCreateMenuBar(main_w, "menubar", NULL, 0);

  // Set OverlayVisual structure for menus
  MenuOverlayVisual * overlay_Visual = 0;
  if(xmvt.overlayVisual) {
    overlay_Visual = new MenuOverlayVisual;
    overlay_Visual->overlayVisual   = xmvt.overlayVisual;
    overlay_Visual->overlayDepth    = xmvt.overlayDepth;
    overlay_Visual->overlayColormap = xmvt.overlayColormap;
    overlay_Visual->popupCallback   =
      (XtCallbackProc) ensurePulldownColormapInstalled;
    overlay_Visual->callback_data   = &xmvt;
  }

  //--------------------------------------------------------------------------
  // First menu is the File menu
  MenuItem Open_menu[] = {
    { "GIO Format", &xmPushButtonGadgetClass, 'G', NULL, NULL, 0,
      dw_file_open_GIO, this, NULL},
    { "_sep1", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "1DDump Format", &xmPushButtonGadgetClass, '1', NULL, NULL, 0,
      dw_file_open_1DDump, this, NULL},
    { "1DDump Abscissa", &xmToggleButtonGadgetClass, '\0', NULL, NULL,
      Abscissa_Set, dw_file_open_1DDAb, this, NULL},
    NULL,
  };
  MenuItem File_menu[] = {
    { "Open...", &xmCascadeButtonGadgetClass, 'n', NULL, NULL,0 ,
      NULL, this, Open_menu},
    { "_sep2", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "Close", &xmPushButtonGadgetClass, 'C', NULL, NULL, 0,
      dw_file_close, this, NULL},
    NULL,
  };
  BuildMenu(menu_bar, XmMENU_PULLDOWN, "File", 'F', overlay_Visual,
	    false, File_menu);

  //--------------------------------------------------------------------------
  // Last menu is the Help menu
  //
  MenuItem Help_menu[] = {
    { "Help", &xmPushButtonGadgetClass, 'H', NULL, NULL,0,
      dw_help, this, NULL},
    NULL,
  };
  Widget Help_m =
    BuildMenu(menu_bar, XmMENU_PULLDOWN, "Help", 'H', overlay_Visual,
	      false, Help_menu);
  XtVaSetValues(menu_bar, XmNmenuHelpWidget, Help_m, NULL);

  XtManageChild(menu_bar);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Frame area to contain OpenGL drawing area
  frame = XtVaCreateManagedWidget("GLframe",xmFrameWidgetClass,
				  main_w,
				  NULL);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // Create Popup menu for frame area of drawing window
  MenuItem Zoom_menu[] = {
    { "Lock Full Frame", &xmToggleButtonGadgetClass, '\0',
      "Ctrl<Key>L", "Ctrl+L", fullframe, dw_LockFullFrame, this, NULL},
    { "_sep3", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "Full Frame", &xmPushButtonGadgetClass, '\0', "Ctrl<Key>F", "Ctrl+F", 0,
      dw_ZoomCurrent, this, NULL},
    { "UnZoom", &xmPushButtonGadgetClass, '\0', "Ctrl<Key>Z", "Ctrl+Z", 0,
      dw_UnZoom, this, NULL},
    { "Reset", &xmPushButtonGadgetClass, '\0', NULL, NULL,0,
      dw_ZoomReset, this, NULL},
    NULL,
  };
  MenuItem Apply_menu[] = {
    { "Copy", &xmPushButtonGadgetClass, '\0', NULL, NULL,0,
      apply_copy, this, NULL},
    { "Abs", &xmPushButtonGadgetClass, '\0', NULL, NULL,0,
      apply_abs, this, NULL},
    { "Log", &xmPushButtonGadgetClass, '\0', NULL, NULL,0,
      apply_log, this, NULL},
    { "Ln", &xmPushButtonGadgetClass, '\0', NULL, NULL, 0,
      apply_ln, this, NULL},
    { "d/dt", &xmPushButtonGadgetClass, '\0', NULL, NULL, 0,
      apply_dt, this, NULL},
    NULL,
  };
  MenuItem Norms_menu[] = {
    { "Linf", &xmPushButtonGadgetClass, '\0', NULL, NULL,0,
      norm_Linf, this, NULL},
    { "L1", &xmPushButtonGadgetClass, '\0', NULL, NULL,0,
      norm_L1, this, NULL},
    { "L2", &xmPushButtonGadgetClass, '\0', NULL, NULL,0,
      norm_L2, this, NULL},
    NULL,
  };
  MenuItem Animate_menu[] = { // If you change this, be sure to check
                              // SetAnimateToggle
    { "Animate", &xmToggleButtonGadgetClass, '\0', "Ctrl<Key>A", "Ctrl+A",
      animate, dw_animate, this, NULL},
    { "_sep3", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "Step Forward", &xmPushButtonGadgetClass, '\0', "<Key>Right", "   ->",
//      { "Step Forward", &xmPushButtonGadgetClass, '\0', "<Key>F", "F",
      0, dw_stepforward, this, NULL},
    { "Step Back", &xmPushButtonGadgetClass, '\0', "<Key>Left", "   <-",
//      { "Step Back", &xmPushButtonGadgetClass, '\0', "<Key>B", "B",
      0, dw_stepbackward, this, NULL},
    { "_sep3", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "Reset", &xmPushButtonGadgetClass, '\0', "Ctrl<Key>R", "Ctrl+R", 0,
      dw_reset_animate, this, NULL},
    NULL,
  };
  MenuItem Popup_menu[] = { // If you change this, be sure to check
                            // SetAnimateToggle and SetFullZoomToggle
    { "Zoom", &xmCascadeButtonGadgetClass, '\0', NULL, NULL, 0,
      NULL, this, Zoom_menu},
    { "_sep3", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "Apply", &xmCascadeButtonGadgetClass, '\0', NULL, NULL, 0,
      NULL, this, Apply_menu},
    { "_sep3", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "Norms", &xmCascadeButtonGadgetClass, '\0', NULL, NULL, 0,
      NULL, this, Norms_menu},
    { "_sep3", &xmSeparatorGadgetClass, '\0', NULL, NULL, 0, NULL, NULL, NULL},
    { "Animate", &xmCascadeButtonGadgetClass, '\0', NULL, NULL, 0,
      NULL, this, Animate_menu},
    NULL,
  };
  popup = BuildMenu(frame, XmMENU_POPUP, "", '\0', overlay_Visual,
		    false, Popup_menu);

  XtAddEventHandler(frame, ButtonPressMask, False, activatePopup, popup);
  delete overlay_Visual;
  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  // OpenGL drawing area
  glx_area = XtVaCreateManagedWidget("glxarea",glwMDrawingAreaWidgetClass,
				     frame,
				     GLwNvisualInfo,xmvt.vi,
				     XmNuserData,this, // For Button actions
				     NULL);
  XtVaGetValues(glx_area, XtNwidth, &viewWidth, XtNheight, &viewHeight, NULL);
//    gcv.foreground = BlackPixelOfScreen(XtScreen(glx_area));
//    gc = XCreateGC(xmvt.display, XtWindow(glx_area), GCForeground, &gcv);
//    XSetFunction(xmvt.display, gc, GXxor);

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
  // Create a Text Widget
  text_area = XtVaCreateManagedWidget("draw_text", xmTextWidgetClass,
				      main_w,
				      XmNmarginHeight, 1,
				      XmNmarginWidth, 1,
				      XmNeditable, FALSE,
				      XmNcursorPositionVisible, FALSE,
				      XmNtraversalOn, FALSE,
				      XmNcolumns, 15,
				      XmNscrollHorizontal, FALSE,
				      XmNscrollVertical, FALSE,
				      NULL);
  XtManageChild(text_area);

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  XmMainWindowSetAreas(main_w,menu_bar,text_area,NULL,NULL,frame);

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

  // Delete from main window list
  XmString wn = XmStringCreateLocalized(name);
  XmListDeleteItem(xmvt.window_list,wn);
  XmStringFree(wn);

  if(xmvt.dw_listed == this) {// Remove info from Data sets list
    xmvt.dw_listed = 0;
    XmListDeleteAllItems(xmvt.dataset_list);
    if(xmvt.ds_listed) {
      xmvt.ds_listed = 0;
      XtVaSetValues(xmvt.info_label, XmNlabelString, xmvt.NULL_String, NULL);
    }
  }

  // *** NOTE *** We are destroying a Shell and not a Widget ????
  // *** Mesa error message ***
  //     "Warning: XtRemoveGrab asked to remove a widget not on the list"
  // cout << "in xvt_drawwin destructor" << endl;
  if(Open_Dialog) XtDestroyWidget(Open_Dialog);
  XtDestroyWidget(draw_shell);
}

void xvt_drawwin::init(const int width, const int height)
{
  glx_win = (GLXDrawable) XtWindow(glx_area);
  glXMakeCurrent(xmvt.display,glx_win,cx);
  vt_drawwin::init(width, height);
  // Set X graphics context for drawing zoom rubber band.
  XGCValues gcv;
  gcv.function = GXinvert;
  gc_RB = XCreateGC(xmvt.display,  (Window) glx_win, GCFunction, &gcv);
}

void xvt_drawwin::draw()
{
  ++swapcount;
  if(!visible) {--swapcount; return;}
  if(draw_RB) glXWaitX(); glXWaitGL();
  glXMakeCurrent(xmvt.Xdisplay(),glx_win,cx);
  vt_drawwin::draw();
  //avoid double swaps
  if(!(--swapcount)) glXSwapBuffers(xmvt.Xdisplay(),glx_win);
  XmTextSetString(text_area,labelbuf);
  if(draw_RB) {glXWaitGL(); DrawRubberBand();}
}

void xvt_drawwin::resize(const int new_width, const int new_height)
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
  if(sync_window) {
    std::list<vt_drawwin *>::iterator p;
    for(p = xmvt.sync_list.begin(); p != xmvt.sync_list.end(); p++)
      if((*p)->sync_window) (*p)->redisplay = true;
  }
  if(!xmvt.redisplayPending) {
    xmvt.redisplayID = XtAppAddWorkProc(xmvt.Xapp(),
					(XtWorkProc) handleRedisplay, &xmvt);
    xmvt.redisplayPending = true;
  }
}

void xvt_drawwin::DrawRubberBand()
{
  XDrawRectangle(xmvt.Xdisplay(), (Window) glx_win, gc_RB,
		 xorg_RB, yorg_RB, xwid_RB, ywid_RB);
}

void xvt_drawwin::SetAnimateToggle(const bool on)
{
  WidgetList popup_elem, anim_elem;
  Widget anim_menu;
  XtVaGetValues(popup, XmNchildren, &popup_elem, NULL);
  XtVaGetValues(popup_elem[6], XmNsubMenuId, &anim_menu, NULL);
  XtVaGetValues(anim_menu, XmNchildren, &anim_elem, NULL);
  XmToggleButtonSetState(anim_elem[0], on, False);

}

void xvt_drawwin::SetFullZoomToggle(const bool on)
{
  WidgetList popup_elem, zoom_elem;
  Widget zoom_menu;
  XtVaGetValues(popup, XmNchildren, &popup_elem, NULL);
  XtVaGetValues(popup_elem[0], XmNsubMenuId, &zoom_menu, NULL);
  XtVaGetValues(zoom_menu, XmNchildren, &zoom_elem, NULL);
  XmToggleButtonSetState(zoom_elem[0], on, False);
  fullframe = on;

}

void xvt_drawwin::Norm(const NormType T) const
{
  // Save the directory so that the new Open will start searching here.
  XmString dir, pattern;
  if(!OpenDialog()) {
    dir = XmStringCopy(search_dir);
    pattern = XmStringCopy(search_pattern);
  } else {
    XtVaGetValues(OpenDialog(),XmNdirectory,&dir,XmNpattern,&pattern,NULL);
  }

  xvt_drawwin * dw = new xvt_drawwin(name , xmvt, dir, pattern);

  XmStringFree(dir);
  XmStringFree(pattern);
  
  std::list<vt_data_series *>::const_iterator p;
  for(p = data_list.begin(); p != data_list.end(); p++)
    dw->Add((*p)->Norm(T));
}
