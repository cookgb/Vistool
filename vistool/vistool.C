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
  : Abscissa_Set(0), Abscissa_Filename(0), Abscissa_Data(0)
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
  if(Abscissa_Set) {
    delete [] Abscissa_Filename;
    delete Abscissa_Data;
  }
}

void vt_mainwin::incrementAnimation()
{
  typedef list<vt_drawwin *>::iterator I_dw;
  I_dw p;
  for(p = draw_list.begin(); p != draw_list.end(); p++) {
    vt_drawwin & dw = **p;
    if(dw.animate) {
      dw.increment();
      dw.draw();
      dw.redraw = false;
    }
  }
}



vt_drawwin::vt_drawwin(const char * n, vt_mainwin & mw)
  : Lb_x(0.), Ub_x(1.), Lb_y(0.), Ub_y(1.),
    mvt(mw), animate(false), redraw(false),
    Abscissa_Set(mw.Abscissa_Set)
{
  name = new char[strlen(n)+1];
  strcpy(name,n);

  if(Abscissa_Set) {
    Abscissa_Filename = new char[strlen(mw.Abscissa_Filename)+1];
    strcpy(Abscissa_Filename,mw.Abscissa_Filename);
    Abscissa_Data = new vt_data_1d(*mw.Abscissa_Data);
  } else {
    Abscissa_Filename = 0;
    Abscissa_Data = 0;
  }

  mw.draw_list.push_back(this);
}

vt_drawwin::~vt_drawwin()
{
  delete [] name;
  if(Abscissa_Set) {
    delete [] Abscissa_Filename;
    delete Abscissa_Data;
  }
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

bool vt_drawwin::ImportFile_GIO(char * file)
{
  GIOquery GIOq;
  if(!GIOq.Read(file)) {
    cerr << "Unsupported file type" << endl;
    return false;
  }
  GIOq.Close();
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
    const int Ns = GIO.NSeries();
    vt_data_series ** vsa = new vt_data_series * [Ns];
    const int Nd = GIO.NDataSets();
    for(int i=0; i<Nd; i++) vsa[i] = new vt_data_series(GIO.Name(i));
    for(int N=0; N<Ns; N++) {
      if(GIO.Read()) {
	const double l = GIO.Label();
	const int ext = GIO.Ext(0);
	double * x = new double[ext];
	const double lb = GIO.Lbound(0);
	const double dx = (GIO.Ubound(0) - lb)/(ext-1);
	for(int ix=0; ix<ext; ix++) x[ix] = lb + ix*dx;
	for(int i=0; i<Nd; i++) {
	  vt_data_1d * vtd = new vt_data_1d(l,ext,x,GIO.Data(i));
	  vsa[i]->Append(vtd);
	}
	delete [] x;
      } else {
	for(int i=0; i<Nd; i++) delete [] vsa[i];
	delete [] vsa;
	return false;
      }
    }
    for(int j=0; j<Nd; j++) Add(vsa[j]);
    delete [] vsa;
    windowReshape(cur_width,cur_height);
    break;
  }
  draw();
  return true;
}

bool vt_drawwin::ImportFile_1DDump(char * filename)
{
  // Open file
  ifstream file;
  file.open(filename,ios::in);
  if(!file.good()) return false;

  // Create new vt_data_series
  vt_data_series *series = new vt_data_series(filename);

  // Read timesteps from file until eof or other error
  int Retrieved_Valid_Timestep = 0;
  while(1) {
    const int dim=1;

    // Time
    double time;
    file.read((char *) &time,sizeof(double)); 
    if(!file.good() || (file.gcount() != sizeof(double))) break;

    // Shape
    int *shape = new int[dim];
    file.read((char *) shape,dim*sizeof(int)); 
    if(!file.good() || (file.gcount() != dim*sizeof(int))) {
      delete[] shape;
      break;
    }

    int i;
    for(i=0;i<dim;i++)
      if(shape[i] <= 0) {
	delete [] shape;
	break;
      }
    
    // Wbox
    double *wbox = new double[2*dim];
    file.read(((char *) wbox),2*dim*sizeof(double));
    if(!file.good() || (file.gcount() != 2*dim*sizeof(double))) {
      delete[] shape;
      delete[] wbox;
      break;
    }
    
    // Data
    int datasize = 1;
    for(i=0;i<dim;i++) datasize *= shape[i];
    double *data = new double[datasize];
    if(!data) {
      delete[] shape;
      delete[] wbox;
      break;
    }
    file.read((char *) data,datasize*sizeof(double));
    if(file.gcount() != datasize*sizeof(double)) {
      delete[] shape;
      delete[] wbox;
      delete[] data;
      break;
    }
    
    // Create a new vt_data_1d and append to series
    if(Abscissa_Set) {
      // Get coords from abscissa
      if(Abscissa_Data->Size() == shape[0]) {
	double *x             = new double[shape[0]];
	const double *xevery2 = Abscissa_Data->Data();
	for(int i=0;i<shape[0];i++) x[i] = xevery2[2*i+1];
	// Now append
	series->Append(new vt_data_1d(time,shape[0],x,data));
	Retrieved_Valid_Timestep++;
	delete[] x;
      }
    } else {
      // Compute coordinates
      double *x        = new double[shape[0]];
      const double dx  = (wbox[1]-wbox[0])/(shape[0]-1);
      for(int i=0;i<shape[0];i++) x[i] = wbox[0] + dx*i;
      // Now append
      series->Append(new vt_data_1d(time,shape[0],x,data));
      Retrieved_Valid_Timestep++;
      delete[] x;
    }
    
    // Clean up
    delete[] data;
    delete[] wbox;
    delete[] shape;
  }

  // Close file
  file.close();

  // If nothing has been added to the series, exit
  if(!Retrieved_Valid_Timestep) return false;

  // Add series to list
  Add(series);
  windowReshape(cur_width,cur_height);

  draw();
  return true;
}

bool vt_drawwin::ImportFile_1DAbs(char * filename)
{
  // Open file
  ifstream file;
  file.open(filename,ios::in);
  if(!file.good()) return false;

  // Read ONE timestep from file
  const int dim=1;

  // Time
  double time;
  file.read((char *) &time,sizeof(double)); 
  if(!file.good() || (file.gcount() != sizeof(double))) {
    file.close();
    return false;
  }
  
  // Shape
  int *shape = new int[dim];
  file.read((char *) shape,dim*sizeof(int)); 
  if(!file.good() || (file.gcount() != dim*sizeof(int))) {
    delete[] shape;
    file.close();
    return false;
  }

  int i;
  for(i=0;i<dim;i++)
    if(shape[i] <= 0) {
      delete [] shape;
      file.close();
      return false;
    }

  // Wbox
  double *wbox = new double[2*dim];
  file.read(((char *) wbox),2*dim*sizeof(double));
  if(!file.good() || (file.gcount() != 2*dim*sizeof(double))) {
    delete[] shape;
    delete[] wbox;
    file.close();
    return false;
  }

  // Coordinate Data
  int datasize = 1;
  for(i=0;i<dim;i++) datasize *= shape[i];
  double *data = new double[datasize];
  if(!data) {
    delete[] shape;
    delete[] wbox;
    file.close();
    return false;
  }
  file.read((char *) data,datasize*sizeof(double));
  if(file.gcount() != datasize*sizeof(double)) {
    delete[] shape;
    delete[] wbox;
    delete[] data;
    file.close();
    return false;
  }
  
  // Compute coordinates
  double *x        = new double[shape[0]];
  const double dx  = (wbox[1]-wbox[0])/(shape[0]-1);
  for(i=0;i<shape[0];i++) x[i] = wbox[0] + dx*i;

  // Set data
  if(Abscissa_Data) delete Abscissa_Data;
  Abscissa_Data = new vt_data_1d(time,shape[0],x,data);

  // Set filename
  if(Abscissa_Filename) delete[] Abscissa_Filename;
  Abscissa_Filename = new char[strlen(filename)+1];
  strcpy(Abscissa_Filename,filename);
  
  // Clean up
  delete[] x;
  delete[] wbox;
  delete[] shape;
  delete[] data;
  file.close();

  return true;
}

void vt_drawwin::init(int width, int height)
{
  glClearColor(0.4, 0.4, 0.4, 0.0);
  cur_width = width;
  cur_height = height;
  windowReshape(width,height);
}

void vt_drawwin::draw()
{
  glClear(GL_COLOR_BUFFER_BIT);
  
  typedef list<vt_data_series *>::iterator I_vd;
  I_vd p;
  for(p = data_list.begin(); p != data_list.end(); p++) {
    vt_data_series & ds = **p;
    ds.Current()->draw();
  }
}

void vt_drawwin::increment()
{  
  typedef list<vt_data_series *>::iterator I_vd;
  I_vd p;
  for(p = data_list.begin(); p != data_list.end(); p++) {
    vt_data_series & ds = **p;
    ds.Increment();
  }
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
  const double xb = 0.025*(Ub_x - Lb_x);
  const double yb = 0.025*(Ub_y - Lb_y);
  gluOrtho2D(Lb_x-xb,Ub_x+xb,Lb_y-yb,Ub_y+yb);
}

void vt_drawwin::Add(vt_data_series * ds)
{
  const double lbx = ds->LBx();
  const double ubx = ds->UBx();
  const double lby = ds->LBy();
  const double uby = ds->UBy();
  if(!(data_list.size())) {
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
  data_list.push_back(ds);
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

vt_data_1d::vt_data_1d(const vt_data_1d & src)
  : vt_data(src.label,src.size)
{
  data = new double[2*size];
  Lb_x = src.Lb_x;
  Ub_x = src.Ub_x;
  Lb_y = src.Lb_y;
  Ub_y = src.Ub_y;
  for(int d=0; d<2*size; d++) data[d] = src.data[d];

}

void vt_data_1d::draw() {
  glColor3d(1.0,1.0,1.0);
  double * p = data;
  glBegin(GL_LINE_STRIP);
  for(int i=0; i<size; i++) {
    const double * x = p++;
    const double * y = p++;
    glVertex2d(*x,*y);
  }
  glEnd();
}

vt_data_series::vt_data_series(const char * n)
  : name(0), current_l(0), current(0)
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
  bool first = false;
  if(!(data.size())) {
    Lb_x = lbx;
    Ub_x = ubx;
    Lb_y = lby;
    Ub_y = uby;
    first = true;
  } else {
    if(lbx < Lb_x) Lb_x = lbx;
    if(ubx > Ub_x) Ub_x = ubx;
    if(lby < Lb_y) Lb_y = lby;
    if(uby > Ub_y) Ub_y = uby;
  }
  data.push_back(d);
  if(first) current = data.begin();
}

void vt_data_series::Increment()
{
  ++current;
  if(current == data.end()) current = data.begin();
  current_l = (**current).Label();
}
