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
  GIObase(const char *const*const names, const int n, const int d);
  ~GIObase();
  //
  int writeheader(fstream *const fs) const;
  int readheader(fstream *const fs);
  int NDataSets() const {return Ndatasets;}
  int Dim() const {return dim;}
  const char * Name(int i) const {return name[i];}
protected:
  void GIOAbort(const char *const reason) const;
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
  int Read(const char *const filename);
  int Key() const {return GIOkey;}
  void Close() {fs->close();}
};

template< class D, class L, int d> class GIOdata : public GIObase {
protected:
  L label;        // Label for each step in the series
  int use_bounds; // flag for use of coordinate bounds
  L LBcoord[d];   // Lower bound of coordinates
  L UBcoord[d];   // Upper bound of coordinates
  int ext[d];     // Extent of the data in each dimension
  const D ** dataw;      // Data for each object (writing)
  D ** datar;      // Data for each object (reading)
public:
  GIOdata();
  GIOdata(const char *const*const names, const int n, const int useb = 0);
  // GIOdata(const char ** names, int n, const L * lb, const L * ub);
  ~GIOdata();
  int Write(fstream *const fs) const;
  int Read(fstream *const fs);
  L Label() const {return label;}
  int CoordsDefined() const {return use_bounds;}
  L Lbound(int i) const {return LBcoord[i];}
  L Ubound(int i) const {return UBcoord[i];}
  int Ext(int i) const {return ext[i];}
  const D * Data(int i) const {return datar[i];}
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
  GIOseries(const char *const filename, int & status);
  GIOseries(const char *const filename, const char *const*const names,
	    const int n, int & status);
  ~GIOseries();
  int Write(const L label, const int *const ext, const D *const*const dat,
	    const int n);
  int Read();
  int Key() const {return GIOkey;}
  int NSeries() const {return Nseries;}
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
  GIOcseries(const char *const filename, int & status);
  GIOcseries(const char *const filename, const char *const*const names,
	     const int n, int & status);
  ~GIOcseries();
  int Write(const L label, const L *const lb, const L *const ub,
	    const int *const ext, const D *const*const dat, const int n);
  int Read();
  int Key() const {return GIOkey;}
  int NSeries() const {return Nseries;}
};

#endif // GIOINCL_H
