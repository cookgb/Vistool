// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef GIOTEMPL_C
#define GIOTEMPL_C

#ifdef __GNUC__
#include <string.h>
#else
#include <strings.h>
#endif
#include <stdlib.h>
#include <fstream.h>

#include "GIOtempl.h"

template< class D, class L, int d> GIOdata<D,L,d>::~GIOdata()
{
  if(data && Ndatasets) {
    for(int i=0; i< Ndatasets; i++)
      if(data[i]) delete [] data[i];
    delete [] data;
  }
}

template< class D, class L, int d> GIOdata<D,L,d>::GIOdata()
  : GIObase(d), label(0), use_bounds(0), data(0)
{
  for(int i=0; i<d; i++) {
    LBcoord[i] = 0;
    UBcoord[i] = 0;
    ext[i]=0;
  }
}

template< class D, class L, int d>
GIOdata<D,L,d>::GIOdata(const char ** names, int n, int useb)
  : GIObase(names, n, d), label(0), use_bounds(useb), data(0)
{
  for(int i=0; i<d; i++) {
    LBcoord[i] = 0;
    UBcoord[i] = 0;
    ext[i]=0;
  }
  data = new D * [Ndatasets];
  for(int j=0; j<Ndatasets; j++) data[j] = 0;
}

template< class D, class L, int d> int GIOdata<D,L,d>::Write(fstream * fs)
{
  fs->write((char *)&label,sizeof(L));
  fs->write((char *)&use_bounds,sizeof(int));
  int size = 1;
  for(int i=0; i<dim; i++) {
    fs->write((char *)(LBcoord+i),sizeof(L));
    fs->write((char *)(UBcoord+i),sizeof(L));
    fs->write((char *)(ext+i),sizeof(int));
    size *= ext[i];
  }
  for(int j=0; j<Ndatasets; j++)
    fs->write((char *)data[j],size*sizeof(D));
  return fs->good();
}

template< class D, class L, int d> int GIOdata<D,L,d>::Read(fstream * fs)
{
  fs->read((char *)&label,sizeof(L));
  fs->read((char *)&use_bounds,sizeof(int));
  int size = 1;
  for(int i=0; i<dim; i++) {
    fs->read((char *)(LBcoord+i),sizeof(L));
    fs->read((char *)(UBcoord+i),sizeof(L));
    fs->read((char *)(ext+i),sizeof(int));
    size *= ext[i];
  }
  for(int j=0; j<Ndatasets; j++) {
    if(data[j]) delete [] data[j];
    if(!(data[j] = new D[size])) GIOAbort("memory");
    fs->read((char *)data[j],size*sizeof(D));
  }
  return 1;
}

template< class D, class L, int d, int KEY>
GIOseries<D,L,d,KEY>::GIOseries(const char * filename, int & status)
  : GIOdata<D,L,d>(), GIOkey(0), fs(0), Nseries(0), Nsseek(0)
{
  status = 0;
  const int sl = strlen(filename);
  if(!(FileName = new char[sl+1])) GIOAbort("memory");
  strcpy(FileName, filename);

  fs = new fstream(FileName, ios::in);
  if(!*fs) GIOAbort("cannot open file for reading");
  
  char buffer[TESTTAGLEN];
  fs->read(buffer,TESTTAGLEN);
  if(strncmp(buffer,GIOTESTTAG,TESTTAGLEN)) GIOAbort("GIO file tag error");

  fs->read((char *)&GIOkey,sizeof(int));
  if(GIOkey != KEY) GIOAbort("GIO file KEY error");

  status = readheader(fs);

  fs->read((char *)&Nseries,sizeof(int));

  status = fs->good();

  data = new D * [Ndatasets];
  for(int i=0; i<Ndatasets; i++) data[i] = 0;
}

template< class D, class L, int d, int KEY> 
GIOseries<D,L,d,KEY>::GIOseries(const char * filename, const char ** names,
				int n, int & status)
  : GIOdata<D,L,d>(names,n), GIOkey(KEY), fs(0), Nseries(0), Nsseek(0)
{
  status = 0;
  const int sl = strlen(filename);
  if(!(FileName = new char[sl+1])) GIOAbort("memory");
  strcpy(FileName, filename);

  fs = new fstream(FileName, ios::out|ios::trunc);
  if(!*fs) GIOAbort("cannot open file for writing");
  
  fs->write(GIOTESTTAG,TESTTAGLEN);

  fs->write((char *)&GIOkey,sizeof(int));

  status = writeheader(fs);

  Nsseek = fs->tellp();
  fs->write((char *)&Nseries,sizeof(int));
  status = fs->good();
}

template< class D, class L, int d, int KEY> 
GIOseries<D,L,d,KEY>::~GIOseries()
{
  if(FileName) delete [] FileName;
  if(*fs) fs->close();
}

template< class D, class L, int d, int KEY> 
int GIOseries<D,L,d,KEY>::Write(L l, int * ex, D ** dat, int n)
{
  if(n != Ndatasets) GIOAbort("incompatible number of objects being written");
  label = l;
  for(int i=0; i<d; i++) ext[i] = ex[i];
  for(int j=0; j<Ndatasets; j++) data[j] = dat[j];

  if(!GIOdata<D,L,d>::Write(fs)) return 0;

  // Don't keep the data since it will be deleated if the variable dies!
  for(int k=0; k<Ndatasets; k++) data[k] = NULL;

  Nseries++;
  fs->seekp(Nsseek);
  fs->write((char *)&Nseries,sizeof(int));
  fs->seekp(0,ios::end);
  return fs->good();
}

template< class D, class L, int d, int KEY> 
int GIOseries<D,L,d,KEY>::Read()
{
  return GIOdata<D,L,d>::Read(fs);
}

template< class D, class L, int d, int KEY>
GIOcseries<D,L,d,KEY>::GIOcseries(const char * filename, int & status)
  : GIOdata<D,L,d>(), GIOkey(0), fs(0), Nseries(0), Nsseek(0)
{
  status = 0;
  const int sl = strlen(filename);
  if(!(FileName = new char[sl+1])) GIOAbort("memory");
  strcpy(FileName, filename);

  fs = new fstream(FileName, ios::in);
  if(!*fs) GIOAbort("cannot open file for reading");
  
  char buffer[TESTTAGLEN];
  fs->read(buffer,TESTTAGLEN);
  if(strncmp(buffer,GIOTESTTAG,TESTTAGLEN)) GIOAbort("GIO file tag error");

  fs->read((char *)&GIOkey,sizeof(int));
  if(GIOkey != KEY) GIOAbort("GIO file KEY error");

  status = readheader(fs);

  fs->read((char *)&Nseries,sizeof(int));

  status = fs->good();

  data = new D * [Ndatasets];
  for(int i=0; i<Ndatasets; i++) data[i] = 0;
}

template< class D, class L, int d, int KEY> 
GIOcseries<D,L,d,KEY>::GIOcseries(const char * filename, const char ** names,
				  int n, int & status)
  : GIOdata<D,L,d>(names,n,1), GIOkey(KEY), fs(0), Nseries(0), Nsseek(0)
{
  status = 0;
  const int sl = strlen(filename);
  if(!(FileName = new char[sl+1])) GIOAbort("memory");
  strcpy(FileName, filename);

  fs = new fstream(FileName, ios::out|ios::trunc);
  if(!*fs) GIOAbort("cannot open file for writing");
  
  fs->write(GIOTESTTAG,TESTTAGLEN);

  fs->write((char *)&GIOkey,sizeof(int));

  status = writeheader(fs);

  Nsseek = fs->tellp();
  fs->write((char *)&Nseries,sizeof(int));
  status = fs->good();
}

template< class D, class L, int d, int KEY> 
GIOcseries<D,L,d,KEY>::~GIOcseries()
{
  if(FileName) delete [] FileName;
  if(*fs) fs->close();
}

template< class D, class L, int d, int KEY> 
int GIOcseries<D,L,d,KEY>::Write(L l, L * lb, L * ub, int * ex,
				 D ** dat, int n)
{
  if(n != Ndatasets) GIOAbort("incompatible number of objects being written");
  label = l;
  for(int i=0; i<d; i++){
    LBcoord[i] = lb[i];
    UBcoord[i] = ub[i];
    ext[i] = ex[i];
  }
  for(int j=0; j<Ndatasets; j++) data[j] = dat[j];

  if(!GIOdata<D,L,d>::Write(fs)) return 0;

  // Don't keep the data since it will be deleated if the variable dies!
  for(int k=0; k<Ndatasets; k++) data[k] = NULL;

  Nseries++;
  fs->seekp(Nsseek);
  fs->write((char *)&Nseries,sizeof(int));
  fs->seekp(0,ios::end);
  return fs->good();
}

template< class D, class L, int d, int KEY> 
int GIOcseries<D,L,d,KEY>::Read()
{
  return GIOdata<D,L,d>::Read(fs);
}

#endif //GIOTEMPL_C
