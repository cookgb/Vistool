// -*- c++ -*-
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef XVISTOOL_H
#define XVISTOOL_H

#include <Xm/MainW.h>

#include "vistool.h"

class xvt_drawwin;

class xvt_mainwin : public vt_mainwin {
private:
  friend void mw_file_cb(Widget, XtPointer, XtPointer);
  friend void mw_openfs_cb(Widget, XtPointer, XtPointer);
  friend class xvt_drawwin;
  Display * display;
  XtAppContext app;
  Widget top_shell;
  Widget main_w;
  Widget menu_bar;
  Widget Open_Dialog;
public:
  xvt_mainwin(int & argc, char ** argv);
  ~xvt_mainwin();
  void Loop() { XtAppMainLoop(app);}
};

class xvt_drawwin : public vt_drawwin {
private:
  xvt_mainwin & xvt;
  Widget draw_shell;
  Widget main_w;
  Widget menu_bar;
  Widget Open_Dialog;
public:
  xvt_drawwin(const char * file, xvt_mainwin & mw);
  ~xvt_drawwin();
  virtual void deleteme() { delete this;}
};

#endif // XVISTOOL_H
