// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef VISTOOL_H
#define VISTOOL_H

#include <list.h>
#include <string.h>

class vt_drawwin;

class vt_mainwin {
public:
  list<vt_drawwin *> draw_list;
  vt_mainwin();
  ~vt_mainwin();
  void incrementAnimation();
};

class vt_drawwin {
protected:
  friend vt_mainwin;
  vt_mainwin & mvt;
  bool animate;
  bool redraw;
public:
  char * name;
  vt_drawwin(const char * n, vt_mainwin & mw);
  ~vt_drawwin();
  void close();
  virtual void deleteme() { delete this;}
  virtual void init(int, int);
  virtual void draw();
  virtual void resize(int, int);
  void windowReshape(int, int);
};

#endif // VISTOOL_H
