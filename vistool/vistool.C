// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <list.h>
#include <algorithm>

#include <GL/gl.h>
#include <GL/glu.h>

#include "vistool.h"

#include "GIO/GIOlib.h"

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

bool vt_mainwin::ImportFile(char * file, vt_drawwin & dw)
{
  GIOquery GIOq;
  if(!GIOq.Read(file)) {
    cerr << "Unsupported file type" << endl;
    return false;
  }
  GIOq.Close;
  if(GIOq.Dim() != 1) {
    cerr << "Unsupported data type" << endl;
    return false;
  }
  switch(GIOq.Key()) {
  case GIO_DOUBLE_SERIES:
    break;
  case GIO_DOUBLE_CSERIES:
    int status;
    GIO_double1DC GIO(file, status);
    if(!status) return false;
    vt_data_series * vsa[GIO.NSeries()];
    const int Nd = GIO.NDataSets();
    for(int i=0; i<Nd; i++) vsa[i] = new vt_data_series(GIO.Name(i));
    const int Ns = GIO.NSeries();
    for(int N=0; N<Ns; N++) {
      if(GIO.Read()) {
	const double l = GIO.Label();
	const int ext = GIO.Ext(0);
	double x[ext];
	const double lb = GIO.Lbound(0);
	const double dx = (GIO.Ubound(0) - lb)/(ext-1);
	for(int ix=0; ix<ext; ix++) x[ix] = lb + ix*dx;
	for(int i=0; i<Nd; i++) {
	  vt_data_1d * vtd = new vt_data_1d(l,ext,x,GIO.Data(i));
	  vsa[i]->Append(vtd);
	}
      } else {
	for(int i=0; i<Nd; i++) delete [] vsa[i];
	return false;
      }
    }
    for(int i=0; i<Nd; i++) dw.Add(vsa[i]);
    break;
  }
  return true;
}

void vt_mainwin::incrementAnimation()
{
  typedef list<vt_drawwin *>::iterator I_dw;
  I_dw p;
  for(p = draw_list.begin(); p != draw_list.end(); p++) {
    vt_drawwin & dw = **p;
    if(dw.animate) {
      cout << "Animate " << dw.name << endl;
      dw.draw();
      dw.redraw = false;
    }
  }
}



vt_drawwin::vt_drawwin(const char * n, vt_mainwin & mw)
  : mvt(mw), animate(false), redraw(false)
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
  I_dw d = std::find(mvt.draw_list.begin(),mvt.draw_list.end(),me);
  if(!(*d)) cerr << "Couldn't remove drawwin from list" << endl;
  mvt.draw_list.erase(d);

  // delete memory
  (*d)->deleteme();
}

void vt_drawwin::init(int width, int height)
{
  glClearColor(0.4, 0.4, 0.4, 0.0);
  windowReshape(width,height);
}

void vt_drawwin::draw()
{
  glClear(GL_COLOR_BUFFER_BIT);
  cout << "Draw window contents of " << name << endl;
}

void vt_drawwin::resize(int new_width, int new_height)
{
  windowReshape(new_width,new_height);
}

void vt_drawwin::windowReshape(int width, int height)
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
}

vt_data::~vt_data()
{
  delete [] data;
}

vt_data_1d::vt_data_1d(double l, int s, const double * x, const double * y)
  : vt_data(l,s)
{
  data = new double[2*s];
  int i=0;
  Lb_x = x[0];
  Ub_x = x[0];
  Lb_y = y[0];
  Ub_y = y[0];
  for(int d=0; d<s; d++) {
    const double lx = x[d];
    if(lx < Lb_x) Lb_x = lx;
    else if(lx > Ub_x) Ub_x = lx;
    data[i++] = lx;
    const double ly = y[d];
    if(ly < Lb_y) Lb_y = ly;
    else if(ly > Ub_y) Ub_y = ly;
    data[i++] = ly;
  }
}

vt_data_series::vt_data_series(const char * n)
  : name(0), current_i(0), current_l(0)
{
  name = new char[strlen(n)+1];
  strcpy(name,n);
}

vt_data_series::~vt_data_series()
{
  typedef list<vt_data *>::iterator I_vd;
  I_vd p;
  while((p = data.begin()) != data.end()) {
    data.erase(p);
    delete [] *p;
  }
}

void vt_data_series::Append(vt_data * d)
{
  const double lbx = d->LBx();
  const double ubx = d->UBx();
  const double lby = d->LBy();
  const double uby = d->UBy();
  if(!(data.size())) {
    Lb_x = lbx;
    Ub_x = ubx;
    Lb_y = lby;
    Ub_y = uby;
  } else {
    if(lbx < Lb_x) Lb_x = lbx;
    if(ubx > Ub_x) Ub_x = ubx;
    if(lby < Lb_y) Lb_y = lby;
    if(uby > Ub_y) Ub_y = uby;
  }
  data.push_back(d);
}
