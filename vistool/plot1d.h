// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef PLOT1D_H
#define PLOT1D_H

#include <vector.h>

template<class T> class XYpair {
public:
  T x;
  T y;
  XYpair() : x((T)0), y((T)0) {}
  XYpair(X,Y) : x(X), y(Y) {}
}

template<class T> class plot1dline {
protected:
  XYpair<T> * data; // the line of data.
  int count;        // how many points are in this line of data.
  T label;          // the label for this line of data (usually time).
  XYpair<T> min;    // lower left corner of the data bounding box.
  XYpair<T> max;    // upper right corner of the data bounding box.
public:
  plot1dline() : data(0), count(0), label((T)0), min(), max() {};
  ~plot1dline() {if(data) delete data;}
  int setdata(XYpair<T> * d, int n, T t);
  int copydata(XYpair<T> * d, int n, T t);
  void draw();
}

template<class T> class plot1dsequence {
protected:
  vector<plot1dline<T> *> sequence; // vector of 1D lines to plot.
  vector<<plot1dline<T> *>::iterator current;
  XYpair<T> min;            // lower left corner of the sequence bounding box.
  XYpair<T> max;            // upper right corner of the sequence bounding box.
public:
  plot1dsequence() : sequence(0), current(0), min(), max() {}
  plot1dsequence(int n) : sequence(n), current(0), min(), max() {}
  ~plot1dsequence() {if(sequence) delete [] sequence;}
  void setminmax();
  void plotnext();
}

#endif // PLOT1D_H
