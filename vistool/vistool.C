// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <list.h>
#include <algorithm>
#include <strstream.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "vistool.h"

#include "GIO/GIOlib.h"

#define LAST_LABEL 1.e30

vt_mainwin::vt_mainwin()
  : drawwin_count(0), dw_listed(0), ds_listed(0),
    Abscissa_Set(0), Abscissa_Filename(0), Abscissa_Data(0)
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
      if(dw.forward_animation) dw.increment();
      else dw.decrement();
      dw.draw();
      dw.redraw = false;
    }
  }
}

char * vt_mainwin::NewDrawWindow()
{
  const int len = 18;
  char * buf = new char[len];
  ostrstream dw(buf,len);
  dw << "Draw Window (" << ++drawwin_count << ")" << ends;
  return buf;
}

char * vt_mainwin::Info_Name(const char * n)
{
  const int len = 128;
  char * buf = new char[len];
  ostrstream info(buf,len);
  info << "file:" << n << ends;
  return buf;
}

char * vt_mainwin::Info_Time(int t)
{
  const int len = 128;
  char * buf = new char[len];
  ostrstream info(buf,len);
  info << "# steps: " << t << ends;
  return buf;
}

char * vt_mainwin::Info_Range(const bounds_2D & b)
{
  const int len = 128;
  char * buf = new char[len];
  ostrstream info(buf,len);
  info << "data bounds: ("
       << b.Lb_x << ", " << b.Lb_y << " -> " << b.Ub_x << ", " << b.Ub_y
       << ")" << ends;
  return buf;
}


vt_drawwin::vt_drawwin(const char * n, vt_mainwin & mw)
  : mvt(mw), redraw(false), animate(false), forward_animation(true),
    Abscissa_Set(mw.Abscissa_Set), selected(false)
{
//    name = new char[strlen(n)+1];
//    strcpy(name,n);
  name = mw.NewDrawWindow();

  Cur_Bounds = new bounds_2D(0.0, 0.0, 1.0, 1.0);
  
  labelbuf = new char[70];
  label = new ostrstream(labelbuf,69);
  label->precision(8);
  label->setf(ios::left,ios::adjustfield);

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


vt_drawwin::vt_drawwin(vt_drawwin & dw)
  : mvt(dw.mvt), redraw(false), animate(false), forward_animation(true),
    Abscissa_Set(dw.Abscissa_Set), selected(false)
{
  name = mvt.NewDrawWindow();

  Cur_Bounds = dw.Cur_Bounds;
  
  labelbuf = new char[70];
  label = new ostrstream(labelbuf,69);
  label->precision(8);
  label->setf(ios::left,ios::adjustfield);

  if(Abscissa_Set) {
    Abscissa_Filename = new char[strlen(dw.Abscissa_Filename)+1];
    strcpy(Abscissa_Filename,dw.Abscissa_Filename);
    Abscissa_Data = new vt_data_1d(*dw.Abscissa_Data);
  } else {
    Abscissa_Filename = 0;
    Abscissa_Data = 0;
  }

  mvt.draw_list.push_back(this);
}

vt_drawwin::~vt_drawwin()
{
  // Delete data list
  list<vt_data_series *>::iterator p;
  while((p = data_list.begin()) != data_list.end()) {
    data_list.erase(p);
    delete *p;
  }
  // Delete name of window
  delete [] name;
  // Delete label text
  if(label) delete label;
  if(labelbuf) delete [] labelbuf;
  // Delete Abscissa data
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
  case GIO_DOUBLE_SERIES: {
    int status;
    GIO_double1D GIO(file, status);
    if(!status) return false;
    const int Ns = GIO.NSeries();
    vt_data_series ** vsa = new vt_data_series * [Ns];
    const int Nd = GIO.NDataSets();
    const bool firstiscoord = (Nd < 2) ? false : true;
    const int datastart = firstiscoord ? 1 : 0;
    for(int i=0; i<Nd; i++) vsa[i] = new vt_data_series(GIO.Name(i), file);
    for(int N=0; N<Ns; N++) {
      if(GIO.Read()) {
	const double l = GIO.Label();
	const int ext = GIO.Ext(0);
	const double * x;
	double * xc;
	if(firstiscoord) {
	  x = GIO.Data(0);
	} else {
	  xc = new double[ext];
	  for(int ix=0; ix<ext; ix++) xc[ix] = ix;
	  x = xc;
	}
	for(int i=datastart; i<Nd; i++) {
	  vt_data_1d * vtd = new vt_data_1d(l,ext,x,GIO.Data(i));
	  vsa[i]->Append(vtd);
	}
	if(!firstiscoord) {
	  delete [] xc;
	}
      } else {
	for(int i=0; i<Nd; i++) delete [] vsa[i];
	delete [] vsa;
	return false;
      }
    }
    for(int j=datastart; j<Nd; j++) Add(vsa[j]);
    delete [] vsa;
    break;
  }
  case GIO_DOUBLE_CSERIES: {
    int status;
    GIO_double1DC GIO(file, status);
    if(!status) return false;
    const int Ns = GIO.NSeries();
    vt_data_series ** vsa = new vt_data_series * [Ns];
    const int Nd = GIO.NDataSets();
    for(int i=0; i<Nd; i++) vsa[i] = new vt_data_series(GIO.Name(i), file);
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
    break;
  }
  }
  return true;
}

bool vt_drawwin::ImportFile_1DDump(char * filename)
{
  // Open file
  ifstream file;
  file.open(filename,ios::in);
  if(!file.good()) return false;

  // Create new vt_data_series
  vt_data_series *series = new vt_data_series(filename,filename);

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
  resize(width,height);
}

void vt_drawwin::draw()
{
  glClear(GL_COLOR_BUFFER_BIT);

  typedef list<vt_data_series *>::iterator I_vd;
  I_vd p;
  bool all_done = true;
  glEnableClientState(GL_VERTEX_ARRAY);
  if(forward_animation)
    for(p = data_list.begin(); p != data_list.end(); p++) {
      vt_data_series & ds = **p;
      if(!ds.Done()) {
	all_done = false;
	if(ds.current_l <= current_l)
	  ds.Current()->draw();
      }
    }
  else
    for(p = data_list.begin(); p != data_list.end(); p++) {
      vt_data_series & ds = **p;
      if(!ds.Done()) {
	all_done = false;
	if(ds.current_l >= current_l)
	  ds.Current()->draw();
      }
    }
  glDisableClientState(GL_VERTEX_ARRAY);
  if(all_done) reset_list();
}

double vt_drawwin::next_label()
{ 
  typedef list<vt_data_series *>::iterator I_vd;
  double next_l = LAST_LABEL;
  I_vd ds;
  for(ds = data_list.begin(); ds != data_list.end(); ds++) {
    const double n = (**ds).Next();
    if(n <= next_l) next_l = n;
  }
  return next_l;
}

double vt_drawwin::previous_label()
{ 
  typedef list<vt_data_series *>::iterator I_vd;
  double prev_l = -LAST_LABEL;
  I_vd ds;
  for(ds = data_list.begin(); ds != data_list.end(); ds++) {
    const double n = (**ds).Previous();
    if(n >= prev_l) prev_l = n;
  }
  return prev_l;
}

void vt_drawwin::increment()
{
  typedef list<vt_data_series *>::iterator I_vd;
  I_vd p;
  if(!forward_animation)
    for(p = data_list.begin(); p != data_list.end(); p++)
      (*p)->Reverse(false, current_l);
  forward_animation = true;
  current_l = next_label();
  Label_Text(true);
  for(p = data_list.begin(); p != data_list.end(); p++) {
    vt_data_series & ds = **p;
    const double n = ds.Next();
    if(n <= current_l)
      ds.Increment();
    else if(n == LAST_LABEL)
      ds.done = true;
  }
}

void vt_drawwin::decrement()
{  
  typedef list<vt_data_series *>::iterator I_vd;
  I_vd p;
  if(forward_animation)
    for(p = data_list.begin(); p != data_list.end(); p++)
      (*p)->Reverse(true, current_l);
  forward_animation = false;
  current_l = previous_label();
  Label_Text(true);
  for(p = data_list.begin(); p != data_list.end(); p++) {
    vt_data_series & ds = **p;
    const double p = ds.Previous();
    if(p >= current_l)
      ds.Decrement();
    else if(p == -LAST_LABEL)
      ds.done = true;
  }
}

void vt_drawwin::reset_list()
{  
  typedef list<vt_data_series *>::iterator I_vd;
  I_vd p;
  if(forward_animation) {
    current_l = LAST_LABEL;
    for(p = data_list.begin(); p != data_list.end(); p++) {
      vt_data_series & ds = **p;
      ds.Reset(forward_animation);
      const double c = ds.current_l;
      if(c < current_l) current_l = c;
    }
  } else {
    current_l = -LAST_LABEL;
    for(p = data_list.begin(); p != data_list.end(); p++) {
      vt_data_series & ds = **p;
      ds.Reset(forward_animation);
      const double c = ds.current_l;
      if(c > current_l) current_l = c;
    }
  }
  Label_Text(true);
  draw();
}

void vt_drawwin::reset_CurrentBounds()
{
  // Clear the bounds stack
  while(!bounds_stack.empty()) {
    delete bounds_stack.top();
    bounds_stack.pop();
  }
  // Reset the draw window to the Default with a boarder.
  double xb = 0.025*(Default_Bounds.Ub_x - Default_Bounds.Lb_x);
  if(xb == 0)
    if(Default_Bounds.Ub_x != 0) xb = 0.025*Default_Bounds.Ub_x;
    else xb = 1.e-2;
  double yb = 0.025*(Default_Bounds.Ub_y - Default_Bounds.Lb_y);
  if(yb == 0)
    if(Default_Bounds.Ub_y != 0) yb = 0.025*Default_Bounds.Ub_y;
    else yb = 1.e-2;
  Cur_Bounds->Lb_x = Default_Bounds.Lb_x-xb;
  Cur_Bounds->Lb_y = Default_Bounds.Lb_y-yb;
  Cur_Bounds->Ub_x = Default_Bounds.Ub_x+xb;
  Cur_Bounds->Ub_y = Default_Bounds.Ub_y+yb;
  resize(cur_width, cur_height);
}

void vt_drawwin::push_CurrentBounds(bounds_2D * new_bounds)
{
  bounds_stack.push(Cur_Bounds);
  Cur_Bounds = new_bounds;
  resize(cur_width, cur_height);
}

void vt_drawwin::pop_CurrentBounds()
{
  if(!bounds_stack.empty()) {
    delete Cur_Bounds;
    Cur_Bounds = bounds_stack.top();
    bounds_stack.pop();
    resize(cur_width, cur_height);
  }
}

void vt_drawwin::resize(int new_width, int new_height)
{
  cur_width = new_width;
  cur_height = new_height;
  windowReshape(new_width,new_height);
}

void vt_drawwin::windowReshape(int width, int height)
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(Cur_Bounds->Lb_x, Cur_Bounds->Ub_x,
	     Cur_Bounds->Lb_y, Cur_Bounds->Ub_y);
}

void vt_drawwin::Add(vt_data_series * ds)
{
  if(!(data_list.size())) Default_Bounds = ds->Bounds();
  else Default_Bounds.Union(ds->Bounds());
  data_list.push_back(ds);
  reset_CurrentBounds();
  reset_list();
  Coords_Text(true);
}

void vt_drawwin::Label_Text(bool add)
{
  label->seekp(0);
  label->width(15);
  (*label) << current_l;
  if(!add) (*label) << ends;
}

void vt_drawwin::Coords_Text(bool add)
{
  if(add) label->seekp(15);
  int p = label->precision();
  label->precision(4);
  (*label) <<   " (" << Cur_Bounds->Lb_x << ", " << Cur_Bounds->Lb_y
	   << " -> " << Cur_Bounds->Ub_x << ", " << Cur_Bounds->Ub_y
	   <<    ")" << ends;
  label->precision(p);
}

void vt_drawwin::Apply(TransformationFunction T)
{
  Default_Bounds = bounds_2D(0.0,0.0,0.0,0.0);
  list<vt_data_series *>::iterator p;
  for(p = data_list.begin(); p != data_list.end(); p++) {
    (*p)->Apply(T);
    Default_Bounds.Union((*p)->Bounds());
  }

  reset_CurrentBounds();
  Coords_Text(true);
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
  double Lb_x = x[0];
  double Ub_x = x[0];
  double Lb_y = y[0];
  double Ub_y = y[0];
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
  bounds = bounds_2D(Lb_x,Lb_y,Ub_x,Ub_y);
}

vt_data_1d::vt_data_1d(const vt_data_1d & src)
  : vt_data(src.label,src.size)
{
  bounds = src.bounds;
  data = new double[2*size];
  for(int d=0; d<2*size; d++) data[d] = src.data[d];
}

void vt_data_1d::draw()
{
  glColor3d(1.0,1.0,1.0);
  glVertexPointer(2,GL_DOUBLE,0,data);
  glDrawArrays(GL_LINE_STRIP,0,size);
//    double * p = data;
//    glBegin(GL_LINE_STRIP);
//    for(int i=0; i<size; i++) {
//      const double * x = p++;
//      const double * y = p++;
//      glVertex2d(*x,*y);
//    }
//    glEnd();
}

vt_data * vt_data_1d::Copy()
{
  return new vt_data_1d(*this);
}

void vt_data_1d::Apply(TransformationFunction T)
{
  int i;
  double Lb_y = data[1];
  double Ub_y;
  switch(T) {
  case Abs :
    Lb_y = fabs(data[1]);
    Ub_y = Lb_y;
    for(i=1; i<2*size; i+=2) {
      const double ly = data[i] = fabs(data[i]);
      if(ly < Lb_y) Lb_y = ly;
      else if(ly > Ub_y) Ub_y = ly;
    }
    bounds.Lb_y = Lb_y;
    bounds.Ub_y = Ub_y;
    break;
  case Log :
    if(data[1] == 0.0) Lb_y = -100.0;
    else Lb_y = log10(data[1]);
    Ub_y = Lb_y;
    for(i=1; i<2*size; i+=2) {
      double ly;
      if(data[i] == 0.0) ly = data[i] = -100.0;
      else ly = data[i] = log10(data[i]);
      if(ly < Lb_y) Lb_y = ly;
      else if(ly > Ub_y) Ub_y = ly;
    }
    bounds.Lb_y = Lb_y;
    bounds.Ub_y = Ub_y;
    break;
  case Ln :
    if(data[1] == 0.0) Lb_y = -100.0;
    else Lb_y = log(data[1]);
    Ub_y = Lb_y;
    for(i=1; i<2*size; i+=2) {
      double ly;
      if(data[i] == 0.0) ly = data[i] = -100.0;
      else ly = data[i] = log(data[i]);
      if(ly < Lb_y) Lb_y = ly;
      else if(ly > Ub_y) Ub_y = ly;
    }
    bounds.Lb_y = Lb_y;
    bounds.Ub_y = Ub_y;
    break;
  default :
    cerr << "Unknown TransformationFunction in vt_data_1d::Apply" << endl;
  }
}

vt_data_series::vt_data_series(const char * n, const char * o)
  : name(0), origin(0), done(false), current_l(0), current(0), selected(false)
{
  name = new char[strlen(n)+1];
  strcpy(name,n);
  origin = new char[strlen(o)+1];
  strcpy(origin,o);
}

vt_data_series::vt_data_series(vt_data_series & vs)
  : done(false), current_l(0), current(0), selected(false)
{
  name = new char[strlen(vs.name)+1];
  strcpy(name,vs.name);
  origin = new char[strlen(vs.origin)+1];
  strcpy(origin,vs.origin);
  
  list<vt_data *>::iterator p;
  for(p = vs.data.begin(); p != vs.data.end(); p++)
    Append((*p)->Copy());
}

vt_data_series::~vt_data_series()
{
  typedef list<vt_data *>::iterator I_vd;
  I_vd p;
  while((p = data.begin()) != data.end()) {
    data.erase(p);
    delete *p;
  }
  if(name) delete[] name;
}

void vt_data_series::Append(vt_data * d)
{
  bool first = false;
  if(!(data.size())) {
    bounds = d->Bounds();
    first = true;
  } else bounds.Union(d->Bounds());
  data.push_back(d);
  if(first) current = data.begin();
}

void vt_data_series::Reverse(bool was_forward, double win_current_l)
{
  if(was_forward) {
    if(!done)
      while(Next() != LAST_LABEL ) {
	if(current_l >= win_current_l) break;
	Increment();
      }
    done = false;
  } else {
    if(!done)
      while(Previous() != -LAST_LABEL ) {
	if(current_l <= win_current_l) break;
	Decrement();
      }
    done = false;
  }
}

double vt_data_series::Next()
{
  list<vt_data *>::iterator next = current;
  ++next;
  if(next != data.end())
    return (**next).Label();
  else
    return LAST_LABEL;
}

double vt_data_series::Previous()
{
  list<vt_data *>::iterator next = current;
  if(next != data.begin()) {
    --next;
    return (**next).Label();
  } else
    return -LAST_LABEL;
}

void vt_data_series::Increment()
{
  ++current;
  if(current == data.end()) {
    done = true;
    current_l = LAST_LABEL;
  } else
    current_l = (**current).Label();
}

void vt_data_series::Decrement()
{
  if(current != data.begin()) {
    --current;
    current_l = (**current).Label();
  } else {
    done = true;
    current_l = -LAST_LABEL;
  }
}

void vt_data_series::Reset(bool forward_animation)
{
  if(forward_animation) {
    current = data.begin();
    done = false;
  } else {
    current = --data.end();
    done = false;
  }
  current_l = (**current).Label();
}

void vt_data_series::Apply(TransformationFunction T)
{
  bounds = bounds_2D(0.0,0.0,0.0,0.0);
  list<vt_data *>::iterator p;
  for(p = data.begin(); p != data.end(); p++) {
    (*p)->Apply(T);
    bounds.Union((*p)->Bounds());
  }
  switch(T) {
  case Abs :
    FunctionName("Abs");
    break;
  case Log :
    FunctionName("Log");
    break;
  case Ln :
    FunctionName("Ln");
    break;
  default :
    cerr << "Unknown TransformationFunction in vt_data_series::Apply" << endl;
  }
}

void vt_data_series::FunctionName(const char * func)
{
  const int len = strlen(func) + strlen(name) + 3;
  char * buf = new char[len];
  ostrstream n(buf,len);
  n << func << "(" << name << ")" << ends;
  delete [] name;
  name = buf;
}
