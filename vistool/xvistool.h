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
  friend xvt_drawwin;
  friend void mapStateChanged(Widget, XtPointer, XEvent *, Boolean *);
  friend void dw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
  friend void mw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
  friend void mw_file_open(Widget, XtPointer, XtPointer);
  friend void mw_openfs_cb(Widget, XtPointer, XtPointer);
  friend void dw_InstallPDColormap(Widget, XtPointer, XtPointer);
  friend void dw_popup_animate(Widget, XtPointer, XtPointer);
  friend void handleAnimate(xvt_mainwin *, XtIntervalId *);
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
  int animateCount;
  int animateHiddenCount;
  XtWorkProcId animateID;
  bool redisplayPending;
  XtWorkProcId redisplayID;
public:
  xvt_mainwin(int & argc, char ** argv);
  ~xvt_mainwin();
  void Loop() { XtAppMainLoop(app);}
  void redisplay();
};

class xvt_drawwin : public vt_drawwin {
private:
  friend xvt_mainwin;
  friend void mapStateChanged(Widget, XtPointer, XEvent *, Boolean *);
  friend void dw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
  friend void dw_popup_animate(Widget, XtPointer, XtPointer);
  friend void dw_init(Widget, XtPointer, XtPointer);
  xvt_mainwin & xmvt;
  Widget draw_shell;
  Widget main_w;
  Widget frame;
  Widget glx_area;
  Widget menu_bar;
  Widget popup;
  WidgetList popuplist;
  Widget Open_Dialog;
  bool redisplay;
  bool visible;
  // OpenGL data
  GLXDrawable glx_win;
  GLXContext cx;
  Dimension viewWidth, viewHeight;
public:
  xvt_drawwin(const char * file, xvt_mainwin & mw);
  ~xvt_drawwin();
  virtual void deleteme() { delete this;}
  virtual void init(int, int);
  virtual void draw();
  virtual void resize(int, int);
  void postRedisplay();
};

#endif // XVISTOOL_H
