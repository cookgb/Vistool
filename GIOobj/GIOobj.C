// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <strings.h>
#include <stdlib.h>
#include <fstream.h>
#include <iostream.h>

#include "GIOobj.h"

GIObase::GIObase(const char ** names, int n, int d)
  : Ndatasets(n), dim(d)
{
  if(!(name = new char * [Ndatasets])) GIOAbort("memory");
  for(int i=0; i<n; i++) {
    const char * namel = names[i];
    const int sl = strlen(namel);
    if(!(name[i] = new char[sl+1])) GIOAbort("memory");
    strcpy(name[i], namel);
  }
}

GIObase::~GIObase()
{
  if(name && Ndatasets) {
    for(int i=0; i< Ndatasets; i++)
      if(name[i]) delete [] name[i];
    delete [] name;
  }
}

void GIObase::writeheader(fstream * fs)
{
  fs->write((char *)&Ndatasets,sizeof(int));
  for(int i=0; i<Ndatasets; i++) fs->write(name[i],strlen(name[i])+1);
  fs->write((char *)&dim,sizeof(int));
}

void GIObase::readheader(fstream * fs)
{
  fs->read((char *)&Ndatasets,sizeof(int));
  if(!(name = new char * [Ndatasets])) GIOAbort("memory");
  for(int i=0; i<Ndatasets; i++) {
    char buffer[128];
    fs->getline(buffer,128,'\0');
    const int sl = strlen(buffer);
    if(!(name[i] = new char[sl+1])) GIOAbort("memory");
    strncpy(name[i],buffer,sl+1);
  }    
  fs->read((char *)&dim,sizeof(int));
}

void GIObase::GIOAbort(const char * reason)
{
  cerr << "ABORT from GIO library : " << reason << endl;
  abort();
}


template< class D, class L> GIOdata<D,L>::~GIOdata()
{
  if(ext) delete [] ext;
  if(data && Ndatasets) {
    for(int i=0; i< Ndatasets; i++)
      if(data[i]) delete [] data[i];
    delete [] data;
  }
}

template< class D, class L> bool GIOdata<D,L>::Write(fstream * fs)
{
  fs->write((char *)&label,sizeof(L));
  int size = 1;
  for(int d=0; d<dim; d++) {
    fs->write((char *)(ext+d),sizeof(int));
    size *= ext[d];
  }
  for(int i=0; i<Ndatasets; i++)
    fs->write((char *)data[i],size*sizeof(D));
  return true;
}

template< class D, class L> bool GIOdata<D,L>::Read(fstream * fs)
{
  fs->read((char *)&label,sizeof(L));
  int size = 1;
  for(int d=0; d<dim; d++) {
    fs->read((char *)(ext+d),sizeof(int));
    size *= ext[d];
  }
  for(int i=0; i<Ndatasets; i++) {
    if(!(data[i] = new D[size])) GIOAbort("memory");
    fs->read((char *)data[i],size*sizeof(D));
  }
  return true;
}

