// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef VISTOOL_H
#define VISTOOL_H

#include <list>
#include <stack>
#include <string>
#include <sstream>
#include <string>

enum FileType {
  TYPE_GIO,
  TYPE_1DDump,
  TYPE_1DAb
};

enum TransformationFunction {
  Abs,
  Log,
  Ln,
};

enum TransformSequence {
  T_Deriv
};

enum NormType {
  Linfinity,
  L1,
  L2
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
  std::list<vt_drawwin *> draw_list;
  std::list<vt_drawwin *> sync_list;
  bool sync_dup;
  vt_mainwin();
  ~vt_mainwin();
  void incrementAnimation();
  char * NewDrawWindow();
  char * Info_Name(const char *const n);
  char * Info_Time(const int t);
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
  std::list<vt_data_series *> data_list;
protected:
  friend class vt_mainwin;
  int cur_width;
  int cur_height;
  bounds_2D Default_Bounds;
  bounds_2D * Cur_Bounds;
  std::stack <bounds_2D *> bounds_stack;
protected:
  vt_mainwin & mvt;
  bool animate;
  bool forward_animation;
  double current_l;
  char * labelbuf;
  std::string label;
  std::string coords;
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
  vt_drawwin(const char *const n, vt_mainwin & mw);
  vt_drawwin(const vt_drawwin & dw);
  ~vt_drawwin();
  void close();
  bool ImportFile_GIO(const char *const file);
  bool ImportFile_1DDump(const char *const file);
  bool ImportFile_1DAbs(const char *const file);
  virtual void deleteme() { delete this;}
  virtual void init(const int, const int);
  virtual void draw();
  double next_label();
  double previous_label();
  void increment();
  void decrement();
  void reset_list();
  bounds_2D AddBoarder(const bounds_2D & b);
  void reset_CurrentBounds();
  void push_CurrentBounds(bounds_2D *const new_bounds);
  void pop_CurrentBounds();
  bounds_2D * Minimum_CurrentBounds();
  virtual void resize(const int, const int);
  void windowReshape(const int, const int);
  void Add(vt_data_series *const ds);
  int Width() const {return cur_width;}
  int Height() const {return cur_height;}
  void Label_Text(const bool add);
  void Coords_Text(const bool add);
  void Update_Label_Buffer() {
    std::ostringstream text; text << label << coords;
    text.str().copy(labelbuf,70,0);
  }
  void Apply(const TransformationFunction T);
  void Apply_Seq(const TransformSequence T);
};

class vt_data {
protected:
  double label;
  int size;
  double *data;
  bounds_2D bounds;
public:
  vt_data(const double l, const int s) : label(l), size(s) {}
  ~vt_data();
  double LBx() const {return bounds.Lb_x;}
  double UBx() const {return bounds.Ub_x;}
  double LBy() const {return bounds.Lb_y;}
  double UBy() const {return bounds.Ub_y;}
  bounds_2D Bounds() const {return bounds;}
  virtual void draw() const = 0;
  virtual vt_data * Copy() const = 0;
  double Label() const {return label;}
  const double *Data() const {return data;};
  int Size() const {return size;};
  virtual void Apply(const TransformationFunction T) = 0;
  virtual void Apply_Seq(const TransformSequence T, vt_data & n) = 0;
  virtual double Norm(const NormType T) const = 0;
};

class vt_data_1d : public vt_data {
protected:
public:
  vt_data_1d(const double l, const int n,
	     const double *const x, const double *const y);
  vt_data_1d(const vt_data_1d & src);
  void draw() const;
  vt_data * Copy() const;
  void Apply(const TransformationFunction T);
  void Apply_Seq(const TransformSequence T, vt_data & n);
  double Norm(const NormType T) const;
};

class vt_data_series {
private:
  friend class vt_drawwin;
protected:
  char * name;
  char * origin;
  bounds_2D bounds;
  bool done;
  double current_l;
  std::list<vt_data *> data;
  std::list<vt_data *>::iterator current;
public:
  bool selected;
public:
  vt_data_series(const char *const n, const char *const o);
  vt_data_series(const vt_data_series & vs);
  ~vt_data_series();
  const char * Name() const {return name;}
  const char * Origin() const {return origin;}
  int NSteps() const {return data.size();}
  double LBx() const {return bounds.Lb_x;}
  double UBx() const {return bounds.Ub_x;}
  double LBy() const {return bounds.Lb_y;}
  double UBy() const {return bounds.Ub_y;}
  bounds_2D Bounds() const {return bounds;}
  void Append(vt_data *const d);
  vt_data * Current() const {return *current;}
  bool Done() const {return done;}
  void Reverse(const bool was_forward, const double win_current_l);
  double Next();
  double Previous();
  void Increment();
  void Decrement();
  void Reset(const bool forward_iteration);
  void Apply(const TransformationFunction T);
  void Apply_Seq(const TransformSequence T);
  void FunctionName(const char *const func);
  vt_data_series * Norm(const NormType T) const;
};

#endif // VISTOOL_H
