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
};

class vt_drawwin {
private:
  vt_mainwin & xvt;
public:
  char * name;
  vt_drawwin(const char * n, vt_mainwin & mw);
  ~vt_drawwin();
  void close();
  virtual void deleteme() { delete this;}
};

#endif // VISTOOL_H
