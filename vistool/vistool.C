// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <list>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "vistool.h"

#include "GIO/GIOlib.h"

#define LAST_LABEL 1.e30

vt_mainwin::vt_mainwin()
  : drawwin_count(0), dw_listed(0), ds_listed(0), sync_dup(false),
    Abscissa_Set(0), Abscissa_Filename(0), Abscissa_Data(0)
{
}

vt_mainwin::~vt_mainwin()
{
  typedef std::list<vt_drawwin *>::iterator I_dw;
  I_dw p;
  while((p = draw_list.begin()) != draw_list.end()) {
    (*p)->deleteme();
    draw_list.erase(p);
  }
  if(Abscissa_Set) {
    delete [] Abscissa_Filename;
    delete Abscissa_Data;
  }
}

void vt_mainwin::incrementAnimation()
{
  typedef std::list<vt_drawwin *>::iterator I_dw;
  I_dw p;
  sync_dup = true; // don't let sync recall increment
  for(p = draw_list.begin(); p != draw_list.end(); p++) {
    vt_drawwin & dw = **p;
    if(dw.animate) {
      if(dw.forward_animation) dw.increment();
      else dw.decrement();
      dw.draw();
    }
  }
  sync_dup = false;
}

char * vt_mainwin::NewDrawWindow()
{
  std::ostringstream dw;
  dw << "Draw Window (" << ++drawwin_count << ")" << std::ends;
  char * buf = new char[dw.str().length()];
  dw.str().copy(buf,std::string::npos,0);
  return buf;
}

char * vt_mainwin::Info_Name(const char *const n)
{
  std::ostringstream info;
  info << "file:" << n << std::ends;
  char * buf = new char[info.str().length()];
  info.str().copy(buf,std::string::npos,0);
  return buf;
}

char * vt_mainwin::Info_Time(const int t)
{
  std::ostringstream info;
  info << "# steps: " << t << std::ends;
  char * buf = new char[info.str().length()];
  info.str().copy(buf,std::string::npos,0);
  return buf;
}

char * vt_mainwin::Info_Range(const bounds_2D & b)
{
  std::ostringstream info;
  info << "data bounds: ("
       << b.Lb_x << ", " << b.Lb_y << " -> " << b.Ub_x << ", " << b.Ub_y
       << ")" << std::ends;
  char * buf = new char[info.str().length()];
  info.str().copy(buf,std::string::npos,0);
  return buf;
}


vt_drawwin::vt_drawwin(const char *const n,  vt_mainwin & mw)
  : mvt(mw), redisplay(false), animate(false), forward_animation(true),
    Abscissa_Set(mw.Abscissa_Set),
    Abscissa_Filename(0), Abscissa_Data(0),
    fullframe(false), selected(false), sync_window(false)
{
//    name = new char[strlen(n)+1];
//    strcpy(name,n);
  name = mw.NewDrawWindow();

  Cur_Bounds = new bounds_2D(0.0, 0.0, 1.0, 1.0);
  
  labelbuf = new char[70];

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


vt_drawwin::vt_drawwin(const vt_drawwin & dw)
  : mvt(dw.mvt), redisplay(false), animate(false), forward_animation(true),
    Abscissa_Set(dw.Abscissa_Set),
    fullframe(false), selected(false), sync_window(false)
{
  name = mvt.NewDrawWindow();

  Cur_Bounds = new bounds_2D(0.0, 0.0, 1.0, 1.0);
  *Cur_Bounds = *(dw.Cur_Bounds);
  
  labelbuf = new char[70];

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
  std::list<vt_data_series *>::iterator p;
  while((p = data_list.begin()) != data_list.end()) {
    delete *p;
    data_list.erase(p);
  }
  // Delete name of window
  delete [] name;
  if(labelbuf) delete [] labelbuf;
  if(Cur_Bounds) delete Cur_Bounds;
  // Delete Abscissa data
  if(Abscissa_Set) {
    delete [] Abscissa_Filename;
    delete Abscissa_Data;
  }
}

void vt_drawwin::close()
{
  // remove me from the draw_list;
  typedef std::list<vt_drawwin *>::iterator I_dw;
  const vt_drawwin * me = this;
  I_dw d = std::find(mvt.draw_list.begin(),mvt.draw_list.end(),me);
  if(!(*d)) std::cerr << "Couldn't remove drawwin from list" << std::endl;
  mvt.draw_list.erase(d);

  // delete memory
  deleteme();
}

bool vt_drawwin::ImportFile_GIO(const char *const file)
{
  GIOquery GIOq;
  if(!GIOq.Read(file)) {
    std::cerr << "Unsupported file type" << std::endl;
    return false;
  }
  GIOq.Close();
  if(GIOq.Dim() != 1) {
    std::cerr << "Unsupported data type" << std::endl;
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

bool vt_drawwin::ImportFile_1DDump(const char *const filename)
{
  // Open file
  std::ifstream file;
  file.open(filename,std::ios_base::in);
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

    // Dimension (must be one for now)
    int testdim;
    file.read((char *) &testdim,sizeof(int));
    if(!file.good() || (file.gcount() != sizeof(int)) || testdim != dim) {
      break;
    }

    // Shape
    int *shape = new int[dim];
    file.read((char *) shape,dim*sizeof(int)); 
    if(!file.good() || (file.gcount() != dim*sizeof(int))) {
      delete[] shape;
      break;
    }

    int i;
    for(i=0;i<dim;i++) {
      if(shape[i] <= 0) {
	delete [] shape;
	break;
      }
    }
    
    // Data
    int datasize = 1;
    for(i=0;i<dim;i++) datasize *= shape[i];
    double *data = new double[datasize];
    if(!data) {
      delete[] shape;
      break;
    }
    file.read((char *) data,datasize*sizeof(double));
    if(file.gcount() != datasize*sizeof(double)) {
      delete[] shape;
      delete[] data;
      break;
    }
    
    // Test to see if data contains insane values
    {
      for(int i=0;i<shape[0];i++) 
	if(!finite(data[i])) {
	  delete[] shape;
	  delete[] data;
	  break;
	}
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
      // Compute coordinates: Assume we go from -1 to +1
      double *x        = new double[shape[0]];
      const double dx  = 2.0/(shape[0]-1);
      for(int i=0;i<shape[0];i++) x[i] = -1.0 + dx*i;
      // Now append
      series->Append(new vt_data_1d(time,shape[0],x,data));
      Retrieved_Valid_Timestep++;
      delete[] x;
    }
    
    // Clean up
    delete[] data;
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

bool vt_drawwin::ImportFile_1DDumpChebGL(const char *const filename)
{
  // Open file
  std::ifstream file;
  file.open(filename,std::ios_base::in);
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

    // Dimension (must be one for now)
    int testdim;
    file.read((char *) &testdim,sizeof(int));
    if(!file.good() || (file.gcount() != sizeof(int)) || testdim != dim) {
      break;
    }

    // Shape
    int *shape = new int[dim];
    file.read((char *) shape,dim*sizeof(int)); 
    if(!file.good() || (file.gcount() != dim*sizeof(int))) {
      delete[] shape;
      break;
    }

    int i;
    for(i=0;i<dim;i++) {
      if(shape[i] <= 0) {
	delete [] shape;
	break;
      }
    }
    
    // Data
    int datasize = 1;
    for(i=0;i<dim;i++) datasize *= shape[i];
    double *data = new double[datasize];
    if(!data) {
      delete[] shape;
      break;
    }
    file.read((char *) data,datasize*sizeof(double));
    if(file.gcount() != datasize*sizeof(double)) {
      delete[] shape;
      delete[] data;
      break;
    }
    
    // Test to see if data contains insane values
    {
      for(int i=0;i<shape[0];i++) 
	if(!finite(data[i])) {
	  delete[] shape;
	  delete[] data;
	  break;
	}
    }

    // Create a new vt_data_1d and append to series
    {
      // Compute coordinates: Assume we go from -1 to +1
      // Use Chebyshev-Gauss-Lobatto points
      double *x = new double[shape[0]];
      for(int i=0;i<shape[0];i++) x[i] = -cos(M_PI*i/(shape[0]-1));
      // Now append
      series->Append(new vt_data_1d(time,shape[0],x,data));
      Retrieved_Valid_Timestep++;
      delete[] x;
    }
    
    // Clean up
    delete[] data;
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

bool vt_drawwin::ImportFile_1DAbs(const char *const filename)
{
  // Open file
  std::ifstream file;
  file.open(filename,std::ios_base::in);
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

  // Dimension (must be one for now)
  int testdim;
  file.read((char *) &testdim,sizeof(int));
  if(!file.good() || (file.gcount() != sizeof(int)) || testdim != dim) {
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
  for(i=0;i<dim;i++) {
    if(shape[i] <= 0) {
      delete [] shape;
      file.close();
      return false;
    }
  }

  // Coordinate Data
  int datasize = 1;
  for(i=0;i<dim;i++) datasize *= shape[i];
  double *data = new double[datasize];
  if(!data) {
    delete[] shape;
    file.close();
    return false;
  }
  file.read((char *) data,datasize*sizeof(double));
  if(file.gcount() != datasize*sizeof(double)) {
    delete[] shape;
    delete[] data;
    file.close();
    return false;
  }
  
  // Compute coordinates: assume range -1 to +1
  double *x        = new double[shape[0]];
  const double dx  = 2.0/(shape[0]-1);
  for(i=0;i<shape[0];i++) x[i] = -1.0 + dx*i;

  // Set data
  if(Abscissa_Data) delete Abscissa_Data;
  Abscissa_Data = new vt_data_1d(time,shape[0],x,data);

  // Set filename
  if(Abscissa_Filename) delete[] Abscissa_Filename;
  Abscissa_Filename = new char[strlen(filename)+1];
  strcpy(Abscissa_Filename,filename);
  
  // Clean up
  delete[] x;
  delete[] shape;
  delete[] data;
  file.close();

  return true;
}

void vt_drawwin::init(const int width, const int height)
{
  glClearColor(0.4, 0.4, 0.4, 0.0);
  resize(width,height);
}

void vt_drawwin::draw()
{
  glClear(GL_COLOR_BUFFER_BIT);

  typedef std::list<vt_data_series *>::iterator I_vd;
  I_vd p;
  bool all_done = true;
  if(fullframe) reset_CurrentBounds();
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
#define ROUNDUP 1.000001
  typedef std::list<vt_data_series *>::iterator I_vd;
  double next_l = LAST_LABEL;
  I_vd ds;
  for(ds = data_list.begin(); ds != data_list.end(); ds++) {
    const double n = (**ds).Next();
    if(n <= next_l) next_l = n; // Strict ordering
  }
  { // allow for roundoff error
    double allow_round = next_l;
    const double next_max = next_l*ROUNDUP;
    for(ds = data_list.begin(); ds != data_list.end(); ds++) {
      const double n = (**ds).Next();
      if(n < next_max && n > allow_round) allow_round = n;
    }
    next_l = allow_round;
  }
  return next_l;
}

double vt_drawwin::previous_label()
{  
#define ROUNDDOWN 0.999999
  typedef std::list<vt_data_series *>::iterator I_vd;
  double prev_l = -LAST_LABEL;
  I_vd ds;
  for(ds = data_list.begin(); ds != data_list.end(); ds++) {
    const double n = (**ds).Previous();
    if(n >= prev_l) prev_l = n; // Strict ordering
  }
  { // allow for roundoff error
    double allow_round = prev_l;
    const double prev_min = prev_l*ROUNDDOWN;
    for(ds = data_list.begin(); ds != data_list.end(); ds++) {
      const double n = (**ds).Previous();
      if(n > prev_min && n < allow_round) allow_round = n;
    }
    prev_l = allow_round;
  }
  return prev_l;
}

void vt_drawwin::increment()
{
  typedef std::list<vt_data_series *>::iterator I_vd;
  I_vd p;
  if(!forward_animation)
    for(p = data_list.begin(); p != data_list.end(); p++)
      (*p)->Reverse(false, current_l);
  forward_animation = true;
  current_l = next_label();
  Label_Text(true);
  Update_Label_Buffer();
  for(p = data_list.begin(); p != data_list.end(); p++) {
    vt_data_series & ds = **p;
    const double n = ds.Next();
    if(n <= current_l)
      ds.Increment();
    else if(n == LAST_LABEL)
      ds.done = true;
  }
  if(!mvt.sync_dup && sync_window) { // repeat on any synced windows
    mvt.sync_dup = true;
    std::list<vt_drawwin *>::iterator p;
    for(p = mvt.sync_list.begin(); p != mvt.sync_list.end(); p++)
      if((*p)->sync_window && (this != *p)) (*p)->increment();
    mvt.sync_dup = false;
  }
}

void vt_drawwin::decrement()
{  
  typedef std::list<vt_data_series *>::iterator I_vd;
  I_vd p;
  if(forward_animation)
    for(p = data_list.begin(); p != data_list.end(); p++)
      (*p)->Reverse(true, current_l);
  forward_animation = false;
  current_l = previous_label();
  Label_Text(true);
  Update_Label_Buffer();
  for(p = data_list.begin(); p != data_list.end(); p++) {
    vt_data_series & ds = **p;
    const double p = ds.Previous();
    if(p >= current_l)
      ds.Decrement();
    else if(p == -LAST_LABEL)
      ds.done = true;
  }
  if(!mvt.sync_dup && sync_window) { // repeat on any synced windows
    mvt.sync_dup = true;
    std::list<vt_drawwin *>::iterator p;
    for(p = mvt.sync_list.begin(); p != mvt.sync_list.end(); p++)
      if((*p)->sync_window && (this != *p)) (*p)->decrement();
    mvt.sync_dup = false;
  }
}

void vt_drawwin::reset_list()
{  
  typedef std::list<vt_data_series *>::iterator I_vd;
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
  Update_Label_Buffer();
  draw();
  if(!mvt.sync_dup && sync_window) { // repeat on any synced windows
    mvt.sync_dup = true;
    std::list<vt_drawwin *>::iterator p;
    for(p = mvt.sync_list.begin(); p != mvt.sync_list.end(); p++)
      if((*p)->sync_window && (this != *p)) (*p)->reset_list();
    mvt.sync_dup = false;
  }
}

bounds_2D vt_drawwin::AddBoarder(const bounds_2D & b)
{
  const double boarder_fraction = 0.025;
  const double size_for_zero = 1.e-2;
  double xb = boarder_fraction*(b.Ub_x - b.Lb_x);
  if(xb == 0.0)
    if(b.Ub_x != 0.0) xb = boarder_fraction*b.Ub_x;
    else xb = size_for_zero;
  double yb = boarder_fraction*(b.Ub_y - b.Lb_y);
  if(yb == 0.0)
    if(b.Ub_y != 0.0) yb = boarder_fraction*b.Ub_y;
    else yb = size_for_zero;
  return bounds_2D(b.Lb_x-xb, b.Lb_y-yb, b.Ub_x+xb, b.Ub_y+yb);
}

void vt_drawwin::reset_CurrentBounds()
{
  // Clear the bounds stack
  while(!bounds_stack.empty()) {
    delete bounds_stack.top();
    bounds_stack.pop();
  }
  if(fullframe) {
    delete Cur_Bounds;
    Cur_Bounds = Minimum_CurrentBounds();
  } else *Cur_Bounds = AddBoarder(Default_Bounds);
  Coords_Text(true);
  Update_Label_Buffer();
  resize(cur_width, cur_height);
}

void vt_drawwin::push_CurrentBounds(bounds_2D *const new_bounds)
{
  bounds_stack.push(Cur_Bounds);
  Cur_Bounds = new_bounds;
  Coords_Text(true);
  Update_Label_Buffer();
  resize(cur_width, cur_height);
}

bounds_2D * vt_drawwin::Minimum_CurrentBounds()
{
  bounds_2D * b = new bounds_2D();
  std::list<vt_data_series *>::iterator p = data_list.begin();
  if(forward_animation) {
    while((p != data_list.end())
	  && ((*p)->done || ((*p)->current_l > current_l))) p++;
    if(p == data_list.end()) p = data_list.begin();
    std::list<vt_data *>::iterator ci = (*p)->current;
    if((*p)->done || ((*p)->current_l > current_l)) --ci;
    *b = (*ci)->Bounds();
    for(p++; p != data_list.end(); p++) {
      ci = (*p)->current;
      if((*p)->done || ((*p)->current_l > current_l)) continue;
      b->Union((*ci)->Bounds());
    }
  } else {
    while((p != data_list.end())
	  && ((*p)->done || ((*p)->current_l < current_l))) p++;
    if(p == data_list.end()) p = data_list.begin();
    std::list<vt_data *>::iterator ci = (*p)->current;
    *b = (*ci)->Bounds();
    for(p++; p != data_list.end(); p++) {
      ci = (*p)->current;
      if((*p)->done || ((*p)->current_l < current_l)) continue;
      b->Union((*ci)->Bounds());
    }
  }
  *b = AddBoarder(*b);
  return b;
}

void vt_drawwin::pop_CurrentBounds()
{
  if(!bounds_stack.empty()) {
    delete Cur_Bounds;
    Cur_Bounds = bounds_stack.top();
    bounds_stack.pop();
    Coords_Text(true);
    Update_Label_Buffer();
    resize(cur_width, cur_height);
  }
}

void vt_drawwin::resize(const int new_width, const int new_height)
{
  cur_width = new_width;
  cur_height = new_height;
  windowReshape(new_width,new_height);
}

void vt_drawwin::windowReshape(const int width, const int height)
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(Cur_Bounds->Lb_x, Cur_Bounds->Ub_x,
	     Cur_Bounds->Lb_y, Cur_Bounds->Ub_y);
}

void vt_drawwin::Add(vt_data_series *const ds)
{
  if(!(data_list.size())) Default_Bounds = ds->Bounds();
  else Default_Bounds.Union(ds->Bounds());
  data_list.push_back(ds);
  reset_list();
  reset_CurrentBounds(); // Needs to be called after reset_list!
}

void vt_drawwin::Label_Text(const bool add)
{
  std::ostringstream text;
  text.precision(8);
  text.setf(std::ios_base::left,std::ios_base::adjustfield);
  text.width(15);
  text << current_l;
  label = text.str();
}

void vt_drawwin::Coords_Text(const bool add)
{
  std::ostringstream text;
  text.precision(4);
  text.setf(std::ios_base::left,std::ios_base::adjustfield);
  text << " (" << Cur_Bounds->Lb_x << ", " << Cur_Bounds->Lb_y
       << " -> " << Cur_Bounds->Ub_x << ", " << Cur_Bounds->Ub_y
       <<    ")" << std::ends;
  coords = text.str();
}
  
void vt_drawwin::Apply(const TransformationFunction T)
{
  std::list<vt_data_series *>::iterator p = data_list.begin();
  bool first = true;
  for(p = data_list.begin(); p != data_list.end(); p++) {
    (*p)->Apply(T);
    if(first) {Default_Bounds = (*p)->Bounds(); first = false;}
    else Default_Bounds.Union((*p)->Bounds());
  }

  reset_CurrentBounds();
}

void vt_drawwin::Apply_Seq(const TransformSequence T)
{
  std::list<vt_data_series *>::iterator p = data_list.begin();
  bool first = true;
  for(p = data_list.begin(); p != data_list.end(); p++) {
    (*p)->Apply_Seq(T);
    if(first) {Default_Bounds = (*p)->Bounds(); first = false;}
    else Default_Bounds.Union((*p)->Bounds());
  }

  reset_CurrentBounds();
}

vt_data::~vt_data()
{
  delete [] data;
}

vt_data_1d::vt_data_1d(const double l, const int s,
		       const double *const x, const double *const y)
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

void vt_data_1d::draw() const
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

vt_data * vt_data_1d::Copy() const
{
  return new vt_data_1d(*this);
}

void vt_data_1d::Apply(const TransformationFunction T)
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
    std::cerr << "Unknown TransformationFunction in vt_data_1d::Apply" 
	      << std::endl;
  }
}

void vt_data_1d::Apply_Seq(const TransformSequence T, vt_data & n)
{
  int i;
  double Lb_y;
  double Ub_y;
  double dtinv;
  double ly;
  switch(T) {
  case T_Deriv :
    dtinv = n.Label() - label; if(dtinv == 0.0) dtinv = 1.0;
    dtinv = 1.0/dtinv;
    Ub_y = Lb_y = (n.Data()[1] - data[1])*dtinv;
    for(i=1; i<2*size; i+=2) {
      data[i] = ly = (n.Data()[i] - data[i])*dtinv;
      if(ly < Lb_y) Lb_y = ly;
      else if(ly > Ub_y) Ub_y = ly;
    }
    bounds.Lb_y = Lb_y;
    bounds.Ub_y = Ub_y;
    break;
  default :
    std::cerr << "Unknown TransformSequence in vt_data_1d::Apply_Seq" 
	      << std::endl;
  }
}

double vt_data_1d::Norm(const NormType T) const
{
  if(size == 1) return data[1];
  
  int i;
  double xl,dat, norm = 0.0;
  switch(T) {
  case Linfinity :
    for(i=1; i<2*size; i+=2) {
      dat = fabs(data[i]);
      if(dat > norm) norm = dat;
    }
    break;
  case L1 :
    xl = data[0];
    for(i=1; i<2*size-1; i+=2) {
      norm += 0.5*fabs(data[i])*(data[i+1] - xl);
      xl = data[i-1];
    }
    norm += 0.5*fabs(data[2*size-1])*(data[2*size-2] - xl);
    break;
  case L2 :
    xl = data[0];
    for(i=1; i<2*size-1; i+=2) {
      dat = data[i];
      norm += 0.5*dat*dat*(data[i+1] - xl);
      xl = data[i-1];
    }
    dat = data[2*size-1];
    norm += 0.5*dat*dat*(data[2*size-2] - xl);
    norm = sqrt(norm);
    break;
  default :
    std::cerr << "Unknown TransformationFunction in vt_data_1d::Apply" 
	      << std::endl;
  }
  return norm;
}

vt_data_series::vt_data_series(const char *const n, const char *const o)
  : name(0), origin(0), done(false), current_l(0), current(0), selected(false)
{
  name = new char[strlen(n)+1];
  strcpy(name,n);
  origin = new char[strlen(o)+1];
  strcpy(origin,o);
}

vt_data_series::vt_data_series(const vt_data_series & vs)
  : done(false), current_l(0), current(0), selected(false)
{
  name = new char[strlen(vs.name)+1];
  strcpy(name,vs.name);
  origin = new char[strlen(vs.origin)+1];
  strcpy(origin,vs.origin);
  
  std::list<vt_data *>::const_iterator p;
  for(p = vs.data.begin(); p != vs.data.end(); p++)
    Append((*p)->Copy());
}

vt_data_series::~vt_data_series()
{
  typedef std::list<vt_data *>::iterator I_vd;
  I_vd p;
  while((p = data.begin()) != data.end()) {
    delete *p;
    data.erase(p);
  }
  if(name) delete[] name;
  if(origin) delete[] origin;
}

void vt_data_series::Append(vt_data *const d)
{
  bool first = false;
  if(!(data.size())) {
    bounds = d->Bounds();
    first = true;
  } else bounds.Union(d->Bounds());
  data.push_back(d);
  if(first) current = data.begin();
}

void vt_data_series::Reverse(const bool was_forward,
			     const double win_current_l)
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
  std::list<vt_data *>::iterator next = current;
  ++next;
  if(next != data.end())
    return (**next).Label();
  else
    return LAST_LABEL;
}

double vt_data_series::Previous()
{
  std::list<vt_data *>::iterator next = current;
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

void vt_data_series::Reset(const bool forward_animation)
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

void vt_data_series::Apply(const TransformationFunction T)
{
  std::list<vt_data *>::iterator p = data.begin();
  bool first = true;
  for(p = data.begin(); p != data.end(); p++) {
    (*p)->Apply(T);
    if(first) {bounds = (*p)->Bounds(); first = false;}
    else bounds.Union((*p)->Bounds());
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
    std::cerr << "Unknown TransformationFunction in vt_data_series::Apply" 
	      << std::endl;
  }
}

void vt_data_series::Apply_Seq(const TransformSequence T)
{
  std::list<vt_data *>::iterator p = data.begin();
  bool first = true;
  for(p = data.begin(); p != --(data.end()); p++) {
    std::list<vt_data *>::iterator n = p;
    (*p)->Apply_Seq(T, **(++n));
    if(first) {bounds = (*p)->Bounds(); first = false;}
    else bounds.Union((*p)->Bounds());
  }
  vt_data * last = data.back();
  data.pop_back();
  delete last;

  switch(T) {
  case T_Deriv :
    FunctionName("d/dt");
    break;
  default :
    std::cerr << "Unknown TransformSequence in vt_data_series::Apply_Seq" 
	      << std::endl;
  }
}

void vt_data_series::FunctionName(const char *const func)
{
  const int len = strlen(func) + strlen(name) + 3;
  std::ostringstream n;
  n << func << "(" << name << ")" << std::ends;
  delete [] name;
  char * buf = new char[len];
  n.str().copy(buf,len,0);
  name = buf;
}

vt_data_series * vt_data_series::Norm(const NormType T) const
{
  vt_data_series * ds = new vt_data_series(Name(),Origin());
  int i;
  int ext = {data.size()};
  double * x = new double[ext];
  double * norm = new double[ext];
  std::list<vt_data *>::const_iterator p;
  for(p = data.begin(), i=0; p != data.end(); p++, i++) {
    x[i] = (*p)->Label();
    norm[i] = (*p)->Norm(T);
  }
  vt_data_1d * vtd = new vt_data_1d(0.0,ext,x,norm);
  ds->Append(vtd);
  switch(T) {
  case Abs :
    ds->FunctionName("Linf-norm");
    break;
  case Log :
    ds->FunctionName("L1-norm");
    break;
  case Ln :
    ds->FunctionName("L2-norm");
    break;
  default :
    std::cerr << "Unknown Norm in vt_data_series::Norm" << std::endl;
  }
  return ds;
}
