// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#include <Xm/MainW.h>
#include <Xm/Frame.h>
#include <Xm/FileSB.h>
#include <X11/GLw/GLwMDrawA.h>

#include <iostream.h>
#include <unistd.h>

#include <map.h>

#include "xvistool.h"

#define DWFILEMENU  XmVaPUSHBUTTON,open,'N',NULL,NULL,\
		    XmVaSEPARATOR,\
		    XmVaPUSHBUTTON,close,'C',NULL,NULL

#define DWHELPMENU  XmVaPUSHBUTTON,help,'H',NULL,NULL

xvt_mainwin * xvt;

void main(int argc, char * argv[])
{
  xvt = new xvt_mainwin(argc,argv);
  xvt->Loop();
}

void mw_file_cb(Widget, XtPointer, XtPointer);
void mw_help_cb(Widget, XtPointer, XtPointer);

xvt_mainwin::xvt_mainwin(int & argc, char ** argv)
  : vt_mainwin(), Open_Dialog(0), vi(0), overlayDepth(0),
    doubleBuffer(true)
{
  XtSetLanguageProc(NULL,NULL,NULL);

  String fallbackResources[] = {
    "*sgiMode: true",     /* Try to enable Indigo Magic look & feel */
    "*useSchemes: all",   /* and SGI schemes. */
    "xvistool.title: Visualization Tool",
    "*main.width: 350",
    "*main.height: 75",
    "*glxarea*width: 300",
    "*glxarea*height: 300",
    NULL
  };

  int config[] = { GLX_DOUBLEBUFFER, GLX_RGBA,
		   GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
		   None };
  int * dblBuf = config  ;
  int *snglBuf = config+1;
  
  // Initialize toolkit and parse command line options.
  top_shell = XtVaAppInitialize(&app,"xvistool",NULL,0,&argc,argv,
				fallbackResources,NULL,NULL);

  display = XtDisplay(top_shell);

  if(!vi) {
    // Find an OpenGL-capable RGB visual
    vi = glXChooseVisual(display,DefaultScreen(display),dblBuf);
    if(!vi) {
      vi = glXChooseVisual(display,DefaultScreen(display),snglBuf);
      if(!vi) XtAppError(app,"no RGB visual");
      doubleBuffer = false;
    }
  }

  // main window contains MenuBar
  main_w = XtVaCreateManagedWidget("main",xmMainWindowWidgetClass,
				    top_shell, NULL);

  // Detect overlay support
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
						      

  // Create a simple MenuBar that contains two menus
  XmString file = XmStringCreateLocalized("File");
  XmString help = XmStringCreateLocalized("Help");
  menu_bar = XmVaCreateSimpleMenuBar(main_w,"menubar",
				     XmVaCASCADEBUTTON, file, 'F',
				     XmVaCASCADEBUTTON, help, 'H',
				     NULL);
  XmStringFree(file);

  // Tell the MenuBar which button is th help menu
  if(Widget w = XtNameToWidget(menu_bar,"button_1"))
    XtVaSetValues(menu_bar,XmNmenuHelpWidget,w,NULL);

  // First menu is the File menu -- callback is file_cb()
  XmString open = XmStringCreateLocalized("Open...");
  XmString quit = XmStringCreateLocalized("Quit");
  XmVaCreateSimplePulldownMenu(menu_bar,"filemenu",0,
			       (XtCallbackProc) mw_file_cb,
			       XmVaPUSHBUTTON,open,'N',NULL,NULL,
			       XmVaSEPARATOR,
			       XmVaPUSHBUTTON,quit,'Q',NULL,NULL,
			       NULL);
  XmStringFree(open);
  XmStringFree(quit);

  // Second menu is the Help menu -- callback is help_cb()
  XmVaCreateSimplePulldownMenu(menu_bar,"helpmenu",1,
			       (XtCallbackProc) mw_help_cb,
			       XmVaPUSHBUTTON,help,'H',NULL,NULL,
			       NULL);
  XmStringFree(help);

  XtManageChild(menu_bar);

  XtRealizeWidget(top_shell);
}


xvt_mainwin::~xvt_mainwin()
{
  // kill all of the X stuff
}

void mw_openfs_cb(Widget, XtPointer, XtPointer);

void mw_file_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  switch((int) client_data)
    {
    case(0) :
      if(!xvt->Open_Dialog) {
	xvt->Open_Dialog = XmCreateFileSelectionDialog(xvt->top_shell,
						       "opendialog",
						       NULL,0);
	XtAddCallback(xvt->Open_Dialog,XmNokCallback,mw_openfs_cb,NULL);
	XtAddCallback(xvt->Open_Dialog,XmNcancelCallback,
		      (XtCallbackProc) XtUnmanageChild,NULL);
      }
      XtManageChild(xvt->Open_Dialog);
      XtPopup(XtParent(xvt->Open_Dialog),XtGrabNone);
      break;
    case(1) :
      delete xvt;
      exit(0);
    default :
      cerr << "Unsupported menu selection in File menu" << endl;
    }
}


void mw_help_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  switch((int) client_data)
    {
    case(0) :
      cout << "Selected Help" << endl;
      break;
    default :
      cerr << "Unsupported menu selection in File menu" << endl;
    }
}


void mw_openfs_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  XmFileSelectionBoxCallbackStruct * fs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  char * file;

  if(fs) {
    if(!XmStringGetLtoR(fs->value,XmFONTLIST_DEFAULT_TAG,&file))
      return; // internal error
    // Create and register the new drawwin
    new xvt_drawwin(file,*xvt);
    XtFree(file);
  }
  XtUnmanageChild(xvt->Open_Dialog);
}

void dw_file_cb(Widget, XtPointer, XtPointer);
void dw_help_cb(Widget, XtPointer, XtPointer);
void dw_InstallPDColormap(Widget, XtPointer, XtPointer);

xvt_drawwin::xvt_drawwin(const char * filename, xvt_mainwin & mw)
  : vt_drawwin(filename,mw), xvt(mw)
{
  draw_shell = XtVaAppCreateShell("xvistool","drawshell",
				  topLevelShellWidgetClass,
				  xvt.display,
				  XmNtitle,filename,
				  NULL);

  // main window contains MenuBar
  main_w = XtVaCreateManagedWidget("drawwin",xmMainWindowWidgetClass,
				   draw_shell,
				   XmNuserData,this,
				   NULL);

  frame = XtVaCreateManagedWidget("frame",xmFrameWidgetClass,
				  main_w,
				  XmNuserData,this,
				  NULL);

  glx_area = XtVaCreateManagedWidget("glxarea",glwMDrawingAreaWidgetClass,
				     frame,
				     GLwNvisualInfo,xvt.vi,
				     XmNuserData,this,
				     NULL);

  // Create a simple MenuBar that contains two menus
  XmString file = XmStringCreateLocalized("File");
  XmString help = XmStringCreateLocalized("Help");
  menu_bar = XmVaCreateSimpleMenuBar(main_w,"menubar",
				     XmVaCASCADEBUTTON, file, 'F',
				     XmVaCASCADEBUTTON, help, 'H',
				     XmNuserData,this,
				     NULL);
  XmStringFree(file);

  // Tell the MenuBar which button is th help menu
  if(Widget w = XtNameToWidget(menu_bar,"button_1"))
    XtVaSetValues(menu_bar,XmNmenuHelpWidget,w,NULL);

  if(xvt.overlayVisual) {
  }

  // First menu is the File menu -- callback is file_cb()
  XmString open = XmStringCreateLocalized("Open...");
  XmString close = XmStringCreateLocalized("Close");
  if(xvt.overlayVisual) {
    Widget menupane =
      XmVaCreateSimplePulldownMenu(menu_bar,"filemenu",0,
				   (XtCallbackProc) dw_file_cb,
				   DWFILEMENU,
				   XmNvisual, xvt.overlayVisual,
				   XmNdepth, xvt.overlayDepth,
				   XmNcolormap, xvt.overlayColormap,
				   XmNuserData,this,
				   NULL);
    XtAddCallback(XtParent(menupane),XmNpopupCallback,
		  (XtCallbackProc) dw_InstallPDColormap,NULL);
  } else {
    XmVaCreateSimplePulldownMenu(menu_bar,"filemenu",0,
				 (XtCallbackProc) dw_file_cb,
				 DWFILEMENU,
				 XmNuserData,this,
				 NULL);
  }
  XmStringFree(open);
  XmStringFree(close);

  // Second menu is the Help menu -- callback is help_cb()
  if(xvt.overlayVisual) {
    Widget menupane =
      XmVaCreateSimplePulldownMenu(menu_bar,"helpmenu",1,
				   (XtCallbackProc) dw_help_cb,
				   DWHELPMENU,
				   XmNvisual, xvt.overlayVisual,
				   XmNdepth, xvt.overlayDepth,
				   XmNcolormap, xvt.overlayColormap,
				   XmNuserData,this,
				   NULL);
  } else {
    XmVaCreateSimplePulldownMenu(menu_bar,"helpmenu",1,
				 (XtCallbackProc) dw_help_cb,
				 DWHELPMENU,
				 XmNuserData,this,
				 NULL);
  }
  XmStringFree(help);

  XtManageChild(menu_bar);

  XmMainWindowSetAreas(main_w,menu_bar,NULL,NULL,NULL,frame);

  XtRealizeWidget(draw_shell);
}


xvt_drawwin::~xvt_drawwin()
{
  XtDestroyWidget(draw_shell);
}
  

//  void dw_openfs_cb(Widget, XtPointer, XtPointer);

void dw_file_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw;
  XtVaGetValues(XtParent(w),XmNuserData,&dw,NULL);

  switch((int) client_data)
    {
    case(0) :
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
      break;
    case(1) :
      if(dw) {
	dw->close();
      } else {
	cerr << "Widge has no xvt_drawwin resource in drawwin close." << endl;
      }
      break;
    default :
      cerr << "Unsupported menu selection in File menu" << endl;
    }
}


void dw_help_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  switch((int) client_data)
    {
    case(0) :
      cout << "Selected Help in a draw window" << endl;
      break;
    default :
      cerr << "Unsupported menu selection in File menu" << endl;
    }
}

void dw_InstallPDColormap(Widget w, XtPointer clientData, XtPointer callData)
{
  // Ensure that overlay pulldown menu's colormap is installed.
  XInstallColormap(xvt->display, xvt->overlayColormap);
}



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
