// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef GIO1D_H
#define GIO1D_H

#ifdef __GNUC__
#pragma interface
#endif

#include <fstream.h>

#include "GIOobj.h"

template< class D, class L> class GIO1dseries : public GIOdata<D,L> {
protected:
  char * FileName;
  fstream * fs;
  int Nseries;
  streampos Nsseek;
public:
  GIO1dseries() : GIOdata(), FileName(0), fs(0), Nseries(0), Nsseek(0) {}
  GIO1dseries(const char * filename, int & status);
  GIO1dseries(const char * filename, const char ** names, int n,
	      int & status);
  ~GIO1dseries();
  int Write(L label, int ext, D ** d, int n);
  int Read();
  int NSeries() {return Nseries;}
};

#endif // GIO1D_H
