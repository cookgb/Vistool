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
#include "xMenu.h"

class xvt_drawwin;

class xvt_mainwin : public vt_mainwin {
private:
  friend xvt_drawwin;
  friend void dw_ensurePulldownColormapInstalled(Widget, XtPointer, XtPointer);
  friend void mw_file_open(Widget, XtPointer, XtPointer);
  friend void dw_openfs_cb(Widget, XtPointer, XtPointer);
  friend void mapStateChanged(Widget, XtPointer, XEvent *, Boolean *);
  friend void dw_animate(Widget, XtPointer, XtPointer);
  friend void dw_stepforward(Widget, XtPointer, XtPointer);
  friend void dw_stepbackward(Widget, XtPointer, XtPointer);
  friend void mw_windowlist(Widget, XtPointer, XtPointer);
  friend void mw_datasetlist(Widget, XtPointer, XtPointer);
  friend void handleAnimate(xvt_mainwin *, XtIntervalId *);
  // X/Motif data
  Display * display;
  XtAppContext app;
  Widget top_shell;
  Widget main_w;
  Widget menu_bar;
  Widget window_list;
  Widget dataset_list;
  Widget info_label;
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
  Widget CheckButton_1DAbs;
  XmString NULL_String;
public:
  xvt_mainwin(int & argc, char ** argv);
  ~xvt_mainwin();
  void Loop() { XtAppMainLoop(app);}
  void redisplay();
public: // utility routines
  XtAppContext Xapp() {return app;}
  Display * Xdisplay() {return display;}
  Colormap XoverlayCM() {return overlayColormap;}
  Widget OpenDialog() {return Open_Dialog;}
  void RegisterDW(const char * window);
};

class xvt_drawwin : public vt_drawwin {
private:
  friend xvt_mainwin;
  friend void mapStateChanged(Widget, XtPointer, XEvent *, Boolean *);
  friend void dw_file_open(Widget, XtPointer, XtPointer);
  friend void dw_openfs_cb(Widget, XtPointer, XtPointer);
  friend void dw_animate(Widget, XtPointer, XtPointer);
  friend void dw_stepforward(Widget, XtPointer, XtPointer);
  friend void dw_stepbackward(Widget, XtPointer, XtPointer);
  friend void Button1DownAction(Widget, XEvent *, String *, Cardinal *);
  friend void Button1MotionAction(Widget, XEvent *, String *, Cardinal *);
  friend void Button1UpAction(Widget, XEvent *, String *, Cardinal *);
  xvt_mainwin & xmvt;
  Widget draw_shell;
  Widget main_w;
  Widget frame;
  Widget glx_area;
  Widget menu_bar;
  Widget text_area;
  Widget popup;
  WidgetList popuplist;
  Widget Open_Dialog;
  XmString search_dir;
  XmString search_pattern;
  bool redisplay;
  bool visible;
  // OpenGL data
  GLXDrawable glx_win;
  GLXContext cx;
  Dimension viewWidth, viewHeight;
  int swapcount;
private:
  GC gc_RB;
  bool draw_RB;
  int xpin_RB, ypin_RB;
  int xorg_RB, yorg_RB;
  int xwid_RB, ywid_RB;
private:
  void CreateWindow();
public:
  Widget CheckButton_1DAbs;
public:
  xvt_drawwin(const char * file, xvt_mainwin & mw,
	      XmString dir, XmString pattern);
  xvt_drawwin(xvt_drawwin & xdw);
  ~xvt_drawwin();
  virtual void deleteme() { delete this;}
  virtual void init(int, int);
  virtual void draw();
  virtual void resize(int, int);
  void postRedisplay();
  void DrawRubberBand();
public:
  Dimension wWidth() {return viewWidth;}
  Dimension wHeight() {return viewHeight;}
  Widget OpenDialog() {return Open_Dialog;}
};

#endif // XVISTOOL_H
