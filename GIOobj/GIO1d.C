// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <string.h>
#include <fstream.h>

#include "GIO1d.h"

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
  status = true;
  
  fs->write(GIOTESTTAG,TESTTAGLEN);

  writeheader(fs);

  Nsseek = fs->tellp();
  fs->write((char *)&Nseries,sizeof(int));
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
  status = true;
  
  char buffer[TESTTAGLEN];
  fs->read(buffer,TESTTAGLEN);
  if(strncmp(buffer,GIOTESTTAG,TESTTAGLEN)) GIOAbort("GIO file tag error");

  readheader(fs);

  fs->read((char *)&Nseries,sizeof(int));

  ext = new int[dim];
  data = new D * [Ndatasets];

  cout << "Ndatasets = " << Ndatasets << endl;
  for(int i=0; i<Ndatasets; i++)
    cout << "name[" << i << "] = " << name[i] << endl;
  cout << "dim = " << dim << endl;
  cout << "Nseries = " << Nseries << endl;
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

  Nseries++;
  fs->seekp(Nsseek);
  fs->write((char *)&Nseries,sizeof(int));
  fs->seekp(0,ios::end);
  return true;
}

template< class D, class L> 
bool GIO1dseries<D,L>::Read()
{
  return GIOdata<D,L>::Read(fs);
}
