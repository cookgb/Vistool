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
#include <strstream.h>

enum FileType {
  TYPE_GIO,
  TYPE_1DDump,
  TYPE_1DAb
};

class vt_drawwin;
class vt_data_1d;

class vt_mainwin {
private:
  int drawwin_count;
public:  // data for 1DDump Abscissa
  bool Abscissa_Set;
  char * Abscissa_Filename;
  vt_data_1d * Abscissa_Data;
public:
  FileType importtype;
  list<vt_drawwin *> draw_list;
  vt_mainwin();
  ~vt_mainwin();
  void incrementAnimation();
  char * NewDrawWindow();
};

class vt_data_series;

class vt_drawwin {
public:
  list<vt_data_series *> data_list;
protected:
  friend vt_mainwin;
  int cur_width;
  int cur_height;
  double Lb_x;
  double Ub_x;
  double Lb_y;
  double Ub_y;
protected:
  vt_mainwin & mvt;
  bool animate;
  bool redraw;
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
  bool selected;
  vt_drawwin(const char * n, vt_mainwin & mw);
  ~vt_drawwin();
  void close();
  bool ImportFile_GIO(char * file);
  bool ImportFile_1DDump(char * file);
  bool ImportFile_1DAbs(char * file);
  virtual void deleteme() { delete this;}
  virtual void init(int, int);
  virtual void draw();
  double next_label();
  void increment();
  void reset_list();
  virtual void resize(int, int);
  void windowReshape(int, int);
  void Add(vt_data_series * ds);
  int Width() {return cur_width;}
  int Height() {return cur_height;}
  void Label_Text(bool add);
  void Coords_Text(bool add);
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
  const double *Data() const {return data;};
  int Size() const {return size;};
};

class vt_data_1d : public vt_data {
protected:
public:
  vt_data_1d(double l, int n, const double * x, const double * y);
  vt_data_1d(const vt_data_1d & src);
  void draw();
};

class vt_data_series {
private:
  friend vt_drawwin;
protected:
  char * name;
  double Lb_x;
  double Ub_x;
  double Lb_y;
  double Ub_y;
  bool done;
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
  bool Done() const {return done;}
  double Next();
  void Increment();
  void Reset();
};

#endif // VISTOOL_H
