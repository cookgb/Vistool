// -*- c++ -*-
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#include <Xm/MainW.h>
#include <Xm/FileSB.h>

#include <iostream.h>
#include <unistd.h>

#include <map.h>

#include "xvistool.h"

xvt_mainwin * xvt;
map<Widget,xvt_drawwin *> widgetmap;

void main(int argc, char * argv[])
{
  xvt = new xvt_mainwin(argc,argv);
  xvt->Loop();
}

void mw_file_cb(Widget, XtPointer, XtPointer);
void mw_help_cb(Widget, XtPointer, XtPointer);

xvt_mainwin::xvt_mainwin(int & argc, char ** argv)
  : vt_mainwin(), Open_Dialog(0)
{
  XtSetLanguageProc(NULL,NULL,NULL);
  
  // Initialize toolkit and parse command line options.
  top_shell = XtVaAppInitialize(&app,"xvistool",NULL,0,&argc,argv,NULL,NULL);

  // main window contains MenuBar
  main_w = XtVaCreateManagedWidget("main",xmMainWindowWidgetClass,
				    top_shell, NULL);

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

  display = XtDisplay(top_shell);

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

xvt_drawwin::xvt_drawwin(const char * filename, xvt_mainwin & mw)
  : vt_drawwin(filename,mw), xvt(mw)
{
  draw_shell = XtVaAppCreateShell(filename,"drawshell",
				  topLevelShellWidgetClass,
				  xvt.display,NULL, NULL);

  // main window contains MenuBar
  main_w = XtVaCreateManagedWidget("drawwin",xmMainWindowWidgetClass,
				    draw_shell, NULL);

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
  XmString close = XmStringCreateLocalized("Close");
  XmVaCreateSimplePulldownMenu(menu_bar,"filemenu",0,
			       (XtCallbackProc) dw_file_cb,
			       XmVaPUSHBUTTON,open,'N',NULL,NULL,
			       XmVaSEPARATOR,
			       XmVaPUSHBUTTON,close,'C',NULL,NULL,
			       NULL);
  XmStringFree(open);
  XmStringFree(close);

  // Second menu is the Help menu -- callback is help_cb()
  XmVaCreateSimplePulldownMenu(menu_bar,"helpmenu",1,
			       (XtCallbackProc) dw_help_cb,
			       XmVaPUSHBUTTON,help,'H',NULL,NULL,
			       NULL);
  XmStringFree(help);

  XtManageChild(menu_bar);

  XtRealizeWidget(draw_shell);

  // register this Widget/xvt_drawwin pair in the widgetmap
  widgetmap[draw_shell] = this;
}


xvt_drawwin::~xvt_drawwin()
{
  XtDestroyWidget(draw_shell);
}
  

//  void dw_openfs_cb(Widget, XtPointer, XtPointer);

void dw_file_cb(Widget w, XtPointer client_data, XtPointer call_data)
{

  // Get top level shell widget associated with event and look up
  // the xvt_drawwin.
  Widget shell = XtParent(XtParent(XtParent(XtParent(XtParent(w)))));
  xvt_drawwin * dw = widgetmap[shell]; 
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
	if(!widgetmap.erase(shell)) 
	  cerr << "Couldn't remove window from lookup table in drawwin close."
	       << endl;
	dw->close();
      } else {
	cerr << "No widgetmap match in drawwin close." << endl;
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
