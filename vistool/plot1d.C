// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include "plot1d.h"

template<class T> int plot1dline<T>::setdata(XYpair<T> * d, int n, T t)
{
  if(n<1) cerr << "no elements in plot1dline<T>::setdata()" << endl;
  if(data) delete data;
  data = d;
  count = n;
  label = t;
  T Xmin = d[0].x;
  T Ymin = d[0].y;
  T Xmax = Xmin;
  T Ymax = Ymin;
  for(int i=1; i<n; i++) {
    const T X = d[i].x;
    const T Y = d[i].y;
    if(X < Xmin) Xmin = X;
    else if(X > Xmax) Xmax = X;
    if(Y < Ymin) Ymin = Y;
    else if(Y > Ymax) Ymax = Y;
  }
  min.x = Xmin; min.y = Ymin;
  max.y = Xmax; max.y = Ymax;
}

template<class T> int plot1dline<T>::copydata(XYpair<T> * d, int n, T t)
{
  if(n<1) cerr << "no elements in plot1dline<T>::copydata()" << endl;
  if(data) delete data;
  data = new XYpair<T>[n];
  count = n;
  label = t;
  T Xmin = d[0].x;
  T Ymin = d[0].y;
  T Xmax = Xmin;
  T Ymax = Ymin;
  data[0].x = Xmin;
  data[0].y = Ymin;
  for(int i=1; i<n; i++) {
    const T X = d[i].x;
    const T Y = d[i].y;
    if(X < Xmin) Xmin = X;
    else if(X > Xmax) Xmax = X;
    data[i].x = X;
    if(Y < Ymin) Ymin = Y;
    else if(Y > Ymax) Ymax = Y;
    data[i].y = Y;
  }
  min.x = Xmin; min.y = Ymin;
  max.y = Xmax; max.y = Ymax;
}

template<class T> void plot1dline<T>::draw()
{
}
