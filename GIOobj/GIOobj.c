// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#ifdef __GNUC__
#pragma implementation
#endif
#include <stdlib.h>
#include <fstream.h>

#include "GIOobj.h"

#ifdef __GNUC__
template class GIOdata<double,double>;
#endif


template< class D, class L> GIOdata<D,L>::~GIOdata()
{
  if(ext) delete [] ext;
  if(data && Ndatasets) {
    for(int i=0; i< Ndatasets; i++)
      if(data[i]) delete [] data[i];
    delete [] data;
  }
}

template< class D, class L> int GIOdata<D,L>::Write(fstream * fs)
{
  fs->write((char *)&label,sizeof(L));
  int size = 1;
  for(int d=0; d<dim; d++) {
    fs->write((char *)(ext+d),sizeof(int));
    size *= ext[d];
  }
  for(int i=0; i<Ndatasets; i++)
    fs->write((char *)data[i],size*sizeof(D));
  return fs->good();
}

template< class D, class L> int GIOdata<D,L>::Read(fstream * fs)
{
  fs->read((char *)&label,sizeof(L));
  int size = 1;
  for(int d=0; d<dim; d++) {
    fs->read((char *)(ext+d),sizeof(int));
    size *= ext[d];
  }
  for(int i=0; i<Ndatasets; i++) {
    if(data[i]) delete [] data[i];
    if(!(data[i] = new D[size])) GIOAbort("memory");
    fs->read((char *)data[i],size*sizeof(D));
  }
  return 1;
}

