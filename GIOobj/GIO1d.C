// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <string.h>
#include <fstream.h>

#pragma implementation
#include "GIO1d.h"

template class GIO1dseries<double,double>;

#define GIOTESTTAG "#GIO UNIQUE TAG\0"
#define TESTTAGLEN 16

template< class D, class L> 
GIO1dseries<D,L>::GIO1dseries(const char * filename, const char ** names,
			      int n, bool & status)
  : GIOdata<D,L>(names,n,1), fs(0), Nseries(0), Nsseek(0)
{
  status = false;
  const int sl = strlen(filename);
  if(!(FileName = new char[sl+1])) GIOAbort("memory");
  strcpy(FileName, filename);

  fs = new fstream(FileName, ios::out|ios::trunc);
  if(!*fs) GIOAbort("cannot open file for writing");
  
  fs->write(GIOTESTTAG,TESTTAGLEN);

  status = writeheader(fs);

  Nsseek = fs->tellp();
  fs->write((char *)&Nseries,sizeof(int));
  status = fs->good();
}

template< class D, class L> 
GIO1dseries<D,L>::GIO1dseries(const char * filename, bool & status)
  : GIOdata<D,L>(), fs(0), Nseries(0), Nsseek(0)
{
  status = false;
  const int sl = strlen(filename);
  if(!(FileName = new char[sl+1])) GIOAbort("memory");
  strcpy(FileName, filename);

  fs = new fstream(FileName, ios::in);
  if(!*fs) GIOAbort("cannot open file for writing");
  
  char buffer[TESTTAGLEN];
  fs->read(buffer,TESTTAGLEN);
  if(strncmp(buffer,GIOTESTTAG,TESTTAGLEN)) GIOAbort("GIO file tag error");

  status = readheader(fs);

  fs->read((char *)&Nseries,sizeof(int));

  status = fs->good();

  ext = new int[dim];
  data = new D * [Ndatasets];
  for(int i=0; i<Ndatasets; i++) data[i] = 0;
}

template< class D, class L> 
GIO1dseries<D,L>::~GIO1dseries()
{
  if(FileName) delete [] FileName;
  if(*fs) fs->close();
}

template< class D, class L> 
bool GIO1dseries<D,L>::Write(L l, int ex, D ** d, int n)
{
  label = l;
  ext[0] = ex;
  if(n != Ndatasets) GIOAbort("incompatible number of objects being written");
  for(int i=0; i<Ndatasets; i++) data[i] = d[i];

  if(!GIOdata<D,L>::Write(fs)) return false;

  // Don't keep the data since it will be deleated if the variable dies!
  for(int i=0; i<Ndatasets; i++) data[i] = NULL;

  Nseries++;
  fs->seekp(Nsseek);
  fs->write((char *)&Nseries,sizeof(int));
  fs->seekp(0,ios::end);
  return fs->good();
}

template< class D, class L> 
bool GIO1dseries<D,L>::Read()
{
  return GIOdata<D,L>::Read(fs);
}
