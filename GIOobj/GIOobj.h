// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef GIOOBJ_H
#define GIOOBJ_H

#include <fstream.h>

class GIObase {
protected:
  int Ndatasets;  // Number of objects in the record
  char ** name;   // Names of the objects in the record
  int dim;        // Dimensionality of the data
public:
  GIObase() : Ndatasets(0), name(0), dim(0) {}
  GIObase(const char ** names, int n, int d);
  ~GIObase();
  //
  void writeheader(fstream * fs);
  void readheader(fstream * fs);
protected:
  void GIOAbort(const char * reason);
};

template< class D, class L> class GIOdata : public GIObase {
protected:
  L label;      // Label for each step in the series
  int * ext;    // Extent of the data in each dimension
  D ** data;    // Data for each object
public:
  GIOdata() : GIObase(), label(0), ext(0), data(0) {}
  GIOdata(const char ** names, int n, int d)
    : GIObase(names, n, d), label(0), ext(0), data(0)
  {
    ext = new int[dim];
    data = new D * [Ndatasets];
  }
  ~GIOdata();
  bool Write(fstream * fs);
  bool Read(fstream * fs);
};

#endif // GIOOBJ_H

