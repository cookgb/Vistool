// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef XVISTOOL_H
#define XVISTOOL_H

#include <Xm/MainW.h>
#include <GL/glx.h>

#include "vistool.h"
#include "sovLayerUtil.h"

class xvt_drawwin;

class xvt_mainwin : public vt_mainwin {
private:
public:
  friend void dw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
  friend void mw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
  friend void mw_file_open(Widget, XtPointer, XtPointer);
  friend void mw_openfs_cb(Widget, XtPointer, XtPointer);
  friend void dw_InstallPDColormap(Widget, XtPointer, XtPointer);
  friend void activateMenu(Widget, XtPointer, XEvent *, Boolean *);
  friend class xvt_drawwin;
  // X/Motif data
  Display * display;
  XtAppContext app;
  Widget top_shell;
  Widget main_w;
  Widget menu_bar;
  Widget Open_Dialog;
  XVisualInfo *vi;
  Visual * overlayVisual;
  int      overlayDepth;
  Colormap overlayColormap;
  bool doubleBuffer;
  // OpenGL data
  GLXContext cx;
public:
  xvt_mainwin(int & argc, char ** argv);
  ~xvt_mainwin();
  void Loop() { XtAppMainLoop(app);}
};

class xvt_drawwin : public vt_drawwin {
private:
public:
  friend void dw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
  friend void activateMenu(Widget, XtPointer, XEvent *, Boolean *);
  xvt_mainwin & xvt;
  Widget draw_shell;
  Widget main_w;
  Widget frame;
  Widget glx_area;
  Widget menu_bar;
  Widget popup;
  WidgetList popuplist;
  Widget Open_Dialog;
public:
  xvt_drawwin(const char * file, xvt_mainwin & mw);
  ~xvt_drawwin();
  virtual void deleteme() { delete this;}
};

#endif // XVISTOOL_H
