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
  bool ImportFile(char * file, vt_drawwin & dw);
  void incrementAnimation();
};

class vt_data_series;

class vt_drawwin {
private:
  list<vt_data_series *> data_list;
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
  void Add(vt_data_series * ds) {data_list.push_back(ds);}
};

class vt_data {
protected:
  double label;
  int size;
  double *data;
  double Lb_x;
  double Ub_x;
  double Lb_y;
  double Ub_y;
public:
  vt_data(double l, int s) : label(l), size(s) {}
  ~vt_data();
  double LBx() {return Lb_x;}
  double UBx() {return Ub_x;}
  double LBy() {return Lb_y;}
  double UBy() {return Ub_y;}
};

class vt_data_1d : public vt_data {
protected:
public:
  vt_data_1d(double l, int n, const double * x, const double * y);
};

class vt_data_series {
protected:
  char * name;
  double Lb_x;
  double Ub_x;
  double Lb_y;
  double Ub_y;
  int current_i;
  double current_l;
  list<vt_data *> data;
public:
  vt_data_series(const char * n);
  ~vt_data_series();
  double LBx() {return Lb_x;}
  double UBx() {return Ub_x;}
  double LBy() {return Lb_y;}
  double UBy() {return Ub_y;}
  void Append(vt_data * d);
};

#endif // VISTOOL_H
