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
  int cur_width;
  int cur_height;
  double Lb_x;
  double Ub_x;
  double Lb_y;
  double Ub_y;
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
  void increment();
  virtual void resize(int, int);
  void windowReshape(int, int);
  void Add(vt_data_series * ds);
  int Width() {return cur_width;}
  int Height() {return cur_height;}
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
  virtual void draw() {}
  double Label() {return label;}
};

class vt_data_1d : public vt_data {
protected:
public:
  vt_data_1d(double l, int n, const double * x, const double * y);
  void draw();
};

class vt_data_series {
protected:
  char * name;
  double Lb_x;
  double Ub_x;
  double Lb_y;
  double Ub_y;
  double current_l;
  list<vt_data *> data;
  list<vt_data *>::iterator current;
public:
  vt_data_series(const char * n);
  ~vt_data_series();
  char * Name() {return name;}
  double LBx() {return Lb_x;}
  double UBx() {return Ub_x;}
  double LBy() {return Lb_y;}
  double UBy() {return Ub_y;}
  void Append(vt_data * d);
  vt_data * Current() {return *current;}
  void Increment();
};

#endif // VISTOOL_H
