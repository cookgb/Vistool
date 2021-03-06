// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <fstream>
#include <iostream>

#include "GIOtempl.h"


GIObase::GIObase(const char *const*const names, const int n, const int d)
  : Ndatasets(n), dim(d)
{
  if(!(name = new char * [Ndatasets])) GIOAbort("memory");
  for(int i=0; i<Ndatasets; i++) {
    const char *const namel = names[i];
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

int GIObase::writeheader(std::fstream *const fs) const
{
  fs->write((char *)&Ndatasets,sizeof(int));
  for(int i=0; i<Ndatasets; i++) fs->write(name[i],strlen(name[i])+1);
  fs->write((char *)&dim,sizeof(int));
  return fs->good();
}

int GIObase::readheader(std::fstream *const fs)
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

void GIObase::GIOAbort(const char *const reason) const
{
  std::cerr << "ABORT from GIO library : " << reason << std::endl;
  abort();
}

GIOquery::~GIOquery()
{
  if(FileName) delete [] FileName;
  fs->close();
  if(fs) delete fs;
}

int GIOquery::Read(const char *const filename)
{
  const int sl = strlen(filename);
  if(FileName) delete [] FileName;
  if(!(FileName = new char[sl+1])) GIOAbort("memory");
  strcpy(FileName, filename);

  fs = new std::fstream(FileName, std::ios_base::in);
  if(!*fs) GIOAbort("cannot open file for reading");
  
  char buffer[TESTTAGLEN];
  fs->read(buffer,TESTTAGLEN);
  if(strncmp(buffer,GIOTESTTAG,TESTTAGLEN)) {
    fs->close();
    return 0;
  }

  fs->read((char *)&GIOkey,sizeof(int));

  return readheader(fs);
}

