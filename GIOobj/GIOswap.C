#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <stdlib.h>

#include "GIOlib.h"

int DumpSwap(istream & in, ostream & out);


void GIOHeader(istream & in, ostream &out,
	       GIOtype & GIOkey, int & Ndatasets, int & dim)
{
  char intbuf[sizeof(int)], cintbuf[sizeof(int)];
  int i,c;

  char buffer[TESTTAGLEN];
  in.read(buffer,TESTTAGLEN);
  if(strncmp(buffer,GIOTESTTAG,TESTTAGLEN)) abort();
  out.write(buffer,TESTTAGLEN);

  in.read(intbuf,sizeof(int));
  for(i=sizeof(int)-1,c=0; i>=0; i--,c++) cintbuf[c] = intbuf[i];
  out.write(cintbuf,sizeof(int));
  GIOkey = *((GIOtype *) cintbuf);

  in.read(intbuf,sizeof(int));
  for(i=sizeof(int)-1,c=0; i>=0; i--,c++) cintbuf[c] = intbuf[i];
  out.write(cintbuf,sizeof(int));
  Ndatasets = *((int *) cintbuf);

  for(i=0; i<Ndatasets; ++i) {
    char buffer[128];
    in.getline(buffer,128,'\0');
    out.write(buffer,strlen(buffer)+1);
  }

  in.read(intbuf,sizeof(int));
  for(i=sizeof(int)-1,c=0; i>=0; i--,c++) cintbuf[c] = intbuf[i];
  out.write(cintbuf,sizeof(int));
  dim = *((int *) cintbuf);
}

void Swap_GIO_double(istream & in, ostream &out, int Ndatasets, int dim)
{
  char intbuf[sizeof(int)], cintbuf[sizeof(int)];
  char doublebuf[sizeof(double)], cdoublebuf[sizeof(double)];
  int i,c;

  int Nseries;
  in.read(intbuf,sizeof(int));
  for(i=sizeof(int)-1,c=0; i>=0; i--,c++) cintbuf[c] = intbuf[i];
  out.write(cintbuf,sizeof(int));
  Nseries = *((int *) cintbuf);
  cerr << "Nseries = " << *((int *) cintbuf) << endl;

  for(int N=0; N<Nseries; ++N) {
    in.read(doublebuf,sizeof(double));
    for(i=sizeof(double)-1,c=0; i>=0; i--,c++) cdoublebuf[c] = doublebuf[i];
    out.write(cdoublebuf,sizeof(double));
    cerr << "Label = " << *((double *) cdoublebuf) << endl;

    in.read(intbuf,sizeof(int));
    for(i=sizeof(int)-1,c=0; i>=0; i--,c++) cintbuf[c] = intbuf[i];
    out.write(cintbuf,sizeof(int));
    cerr << "Use bounds = " << *((int *) cintbuf) << endl;

    int size = 1;
    for(int d=0; d<dim; ++d) {
      in.read(doublebuf,sizeof(double));
      for(i=sizeof(double)-1,c=0; i>=0; i--,c++) cdoublebuf[c] = doublebuf[i];
      out.write(cdoublebuf,sizeof(double));
      cerr << "Lb[" << d << "] = " << *((double *) cdoublebuf) << endl;

      in.read(doublebuf,sizeof(double));
      for(i=sizeof(double)-1,c=0; i>=0; i--,c++) cdoublebuf[c] = doublebuf[i];
      out.write(cdoublebuf,sizeof(double));
      cerr << "Ub[" << d << "] = " << *((double *) cdoublebuf) << endl;
      
      in.read(intbuf,sizeof(int));
      for(i=sizeof(int)-1,c=0; i>=0; i--,c++) cintbuf[c] = intbuf[i];
      out.write(cintbuf,sizeof(int));
      size *= *((int *) cintbuf);
      cerr << "ext[" << d << "] = " << *((int *) cintbuf) << endl;
    }
    cerr << "size = " << size << endl;

    for(int nd=0; nd<size*Ndatasets; ++nd) {
      in.read(doublebuf,sizeof(double));
      for(i=sizeof(double)-1,c=0; i>=0; i--,c++) cdoublebuf[c] = doublebuf[i];
      out.write(cdoublebuf,sizeof(double));
    }
  }
}

void main()
{
  GIOtype GIOkey;
  int Ndatasets, dim;
  GIOHeader(cin, cout, GIOkey, Ndatasets, dim);
  cerr << "GIOkey = " << GIOkey << endl;
  cerr << "Ndatasets = " << Ndatasets << endl;
  cerr << "dim = " << dim << endl;
  switch(GIOkey) {
  case GIO_DOUBLE_SERIES:
  case GIO_DOUBLE_CSERIES:
    Swap_GIO_double(cin,cout,Ndatasets,dim);
    break;
  default:
    cerr << "Unsupported GIO type" << endl;
    abort();
  }
  //  while(GIOSwap(cin,cout));
}






