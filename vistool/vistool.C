// -*- c++ -*-
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <list.h>
#include <algorithm>

#include "vistool.h"

vt_mainwin::vt_mainwin()
{
}

vt_mainwin::~vt_mainwin()
{
  typedef list<vt_drawwin *>::iterator I_dw;
  I_dw p;
  while((p = draw_list.begin()) != draw_list.end()) {
    draw_list.erase(p);
    (*p)->deleteme();
  }
}

vt_drawwin::vt_drawwin(const char * n, vt_mainwin & mw)
  : xvt(mw)
{
  name = new char[strlen(n)+1];
  strcpy(name,n);
  mw.draw_list.push_back(this);
}

vt_drawwin::~vt_drawwin()
{
  delete name;
}
void vt_drawwin::close()
{
  // remove me from the draw_list;
  typedef list<vt_drawwin *>::iterator I_dw;
  const vt_drawwin * me = this;
  I_dw d = std::find(xvt.draw_list.begin(),xvt.draw_list.end(),me);
  if(!(*d)) cerr << "Couldn't remove drawwin from list" << endl;
  xvt.draw_list.erase(d);

  // delete memory
  (*d)->deleteme();
}
