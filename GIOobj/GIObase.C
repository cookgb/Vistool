// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#ifdef __GNUC__
#pragma implementation
#include <string.h>
#else
#include <strings.h>
#endif
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

int GIObase::writeheader(fstream * fs)
{
  fs->write((char *)&Ndatasets,sizeof(int));
  for(int i=0; i<Ndatasets; i++) fs->write(name[i],strlen(name[i])+1);
  fs->write((char *)&dim,sizeof(int));
  return fs->good();
}

int GIObase::readheader(fstream * fs)
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
  return fs->good();
}

void GIObase::GIOAbort(const char * reason)
{
  cerr << "ABORT from GIO library : " << reason << endl;
  abort();
}

