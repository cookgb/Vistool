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
#include <stack.h>
#include <string.h>
#include <strstream.h>

enum FileType {
  TYPE_GIO,
  TYPE_1DDump,
  TYPE_1DAb
};

enum TransformationFunction {
  Abs,
  Log,
  Ln
};

class vt_drawwin;
class vt_data_series;
class vt_data_1d;
class bounds_2D;

class vt_mainwin {
private:
  int drawwin_count;
public:  // pointers to data with information listed in main window
  vt_drawwin     * dw_listed;
  vt_data_series * ds_listed;
public:  // data for 1DDump Abscissa
  bool Abscissa_Set;
  char * Abscissa_Filename;
  vt_data_1d * Abscissa_Data;
public:
  FileType importtype;
  list<vt_drawwin *> draw_list;
  list<vt_drawwin *> sync_list;
  bool sync_dup;
  vt_mainwin();
  ~vt_mainwin();
  void incrementAnimation();
  char * NewDrawWindow();
  char * Info_Name(const char * n);
  char * Info_Time(int t);
  char * Info_Range(const bounds_2D & b);
};


class bounds_2D {
public:
  double Lb_x;
  double Lb_y;
  double Ub_x;
  double Ub_y;
public:
  bounds_2D(double lx = 0.0, double ly = 0.0,
	    double ux = 1.0, double uy = 1.0) : Lb_x(lx), Lb_y(ly),
						Ub_x(ux), Ub_y(uy) {}
  void Union(const bounds_2D & u) {
    if(u.Lb_x < Lb_x) Lb_x = u.Lb_x; if(u.Lb_y < Lb_y) Lb_y = u.Lb_y;
    if(u.Ub_x > Ub_x) Ub_x = u.Ub_x; if(u.Ub_y > Ub_y) Ub_y = u.Ub_y;
  }
};

class vt_data_series;

class vt_drawwin {
public:
  list<vt_data_series *> data_list;
protected:
  friend vt_mainwin;
  int cur_width;
  int cur_height;
  bounds_2D Default_Bounds;
  bounds_2D * Cur_Bounds;
  stack <bounds_2D *> bounds_stack;
protected:
  vt_mainwin & mvt;
  bool animate;
  bool forward_animation;
  double current_l;
  ostrstream * label;
  char * labelbuf;
public:  // data for 1DDump Abscissa
  bool Abscissa_Set;
  char * Abscissa_Filename;
  vt_data_1d * Abscissa_Data;
public:
  FileType importtype;
  char * name;
  bool fullframe;
  bool selected;
  bool sync_window;
  bool redisplay;
  vt_drawwin(const char * n, vt_mainwin & mw);
  vt_drawwin(vt_drawwin & dw);
  ~vt_drawwin();
  void close();
  bool ImportFile_GIO(char * file);
  bool ImportFile_1DDump(char * file);
  bool ImportFile_1DAbs(char * file);
  virtual void deleteme() { delete this;}
  virtual void init(int, int);
  virtual void draw();
  double next_label();
  double previous_label();
  void increment();
  void decrement();
  void reset_list();
  bounds_2D AddBoarder(const bounds_2D b);
  void reset_CurrentBounds();
  void push_CurrentBounds(bounds_2D * new_bounds);
  void pop_CurrentBounds();
  bounds_2D * Minimum_CurrentBounds();
  virtual void resize(int, int);
  void windowReshape(int, int);
  void Add(vt_data_series * ds);
  int Width() const {return cur_width;}
  int Height() const {return cur_height;}
  void Label_Text(bool add);
  void Coords_Text(bool add);
  void Apply(TransformationFunction T);
};

class vt_data {
protected:
  double label;
  int size;
  double *data;
  bounds_2D bounds;
public:
  vt_data(double l, int s) : label(l), size(s) {}
  ~vt_data();
  double LBx() const {return bounds.Lb_x;}
  double UBx() const {return bounds.Ub_x;}
  double LBy() const {return bounds.Lb_y;}
  double UBy() const {return bounds.Ub_y;}
  bounds_2D Bounds() const {return bounds;}
  virtual void draw() = 0;
  virtual vt_data * Copy() = 0;
  double Label() const {return label;}
  const double *Data() const {return data;};
  int Size() const {return size;};
  virtual void Apply(TransformationFunction T) = 0;
};

class vt_data_1d : public vt_data {
protected:
public:
  vt_data_1d(double l, int n, const double * x, const double * y);
  vt_data_1d(const vt_data_1d & src);
  void draw();
  vt_data * Copy();
  void Apply(TransformationFunction T);
};

class vt_data_series {
private:
  friend vt_drawwin;
protected:
  char * name;
  char * origin;
  bounds_2D bounds;
  bool done;
  double current_l;
  list<vt_data *> data;
  list<vt_data *>::iterator current;
public:
  bool selected;
public:
  vt_data_series(const char * n, const char * o);
  vt_data_series(vt_data_series & vs);
  ~vt_data_series();
  char * Name() {return name;}
  char * Origin() {return origin;}
  int NSteps() {return data.size();}
  double LBx() {return bounds.Lb_x;}
  double UBx() {return bounds.Ub_x;}
  double LBy() {return bounds.Lb_y;}
  double UBy() {return bounds.Ub_y;}
  bounds_2D Bounds() const {return bounds;}
  void Append(vt_data * d);
  vt_data * Current() const {return *current;}
  bool Done() const {return done;}
  void Reverse(bool was_forward, double win_current_l);
  double Next();
  double Previous();
  void Increment();
  void Decrement();
  void Reset(bool forward_iteration);
  void Apply(TransformationFunction T);
  void FunctionName(const char * func);
};

#endif // VISTOOL_H
