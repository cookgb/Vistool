// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef GIOINCL_H
#define GIOINCL_H

#include <fstream.h>

#define GIOTESTTAG "#GIO UNIQUE TAG\0"
#define TESTTAGLEN 16

class GIObase {
protected:
  int Ndatasets;  // Number of objects in the record
  char ** name;   // Names of the objects in the record
  int dim;        // Dimensionality of the data
public:
  GIObase(int d = 0) : Ndatasets(0), name(0), dim(d) {}
  GIObase(const char ** names, int n, int d);
  ~GIObase();
  //
  int writeheader(fstream * fs);
  int readheader(fstream * fs);
  int NDataSets() {return Ndatasets;}
  int Dim() {return dim;}
  const char * Name(int i) {return name[i];}
protected:
  void GIOAbort(const char * reason);
};

class GIOquery : public GIObase {
protected:
  int GIOkey;
  char * FileName;
  fstream * fs;
public:
  GIOquery() : GIObase(), GIOkey(0), FileName(0), fs(0) {}
  ~GIOquery();
  //
  int Read(const char * filename);
  int Key() {return GIOkey;}
  void Close() {fs->close();}
};

template< class D, class L, int d> class GIOdata : public GIObase {
protected:
  L label;        // Label for each step in the series
  int use_bounds; // flag for use of coordinate bounds
  L LBcoord[d];   // Lower bound of coordinates
  L UBcoord[d];   // Upper bound of coordinates
  int ext[d];     // Extent of the data in each dimension
  D **data;       // Data for each object
public:
  GIOdata();
  GIOdata(const char ** names, int n, int useb = 0);
  GIOdata(const char ** names, int n, L * lb, L * ub);
  ~GIOdata();
  int Write(fstream * fs);
  int Read(fstream * fs);
  L Label() {return label;}
  int CoordsDefined() {return use_bounds;}
  L Lbound(int i) {return LBcoord[i];}
  L Ubound(int i) {return UBcoord[i];}
  int Ext(int i) {return ext[i];}
  const D * Data(int i) {return data[i];}
};

template< class D, class L, int d, int KEY>
class GIOseries : public GIOdata<D,L,d> {
protected:
  int GIOkey;
  char * FileName;
  fstream * fs;
  int Nseries;
  streampos Nsseek;
public:
  GIOseries()
    : GIOdata<D,L,d>(), GIOkey(0), FileName(0), fs(0), Nseries(0), Nsseek(0) {}
  GIOseries(const char * filename, int & status);
  GIOseries(const char * filename, const char ** names, int n,
	    int & status);
  ~GIOseries();
  int Write(L label, int * ext, D ** dat, int n);
  int Read();
  int Key() {return GIOkey;}
  int NSeries() {return Nseries;}
};

template< class D, class L, int d, int KEY>
class GIOcseries : public GIOdata<D,L,d> {
protected:
  int GIOkey;
  char * FileName;
  fstream * fs;
  int Nseries;
  streampos Nsseek;
public:
  GIOcseries()
    : GIOdata<D,L,d>(), GIOkey(0), FileName(0), fs(0), Nseries(0), Nsseek(0) {}
  GIOcseries(const char * filename, int & status);
  GIOcseries(const char * filename, const char ** names, int n,
	    int & status);
  ~GIOcseries();
  int Write(L label, L * lb, L * ub, int * ext, D ** dat, int n);
  int Read();
  int Key() {return GIOkey;}
  int NSeries() {return Nseries;}
};

#endif // GIOINCL_H









