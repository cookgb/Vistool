// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <iostream.h>
#include <stdlib.h>
#include <math.h>

#include "GIOlib.h"


double wave(double t, double x)
{
//    return 1.5*sin(t - x);
  return 0.05*sin(t - x);
}
double wave2(double t, double x)
{
//    return sin(t + x);
  return 0.01*sin(t + x);
}

int main()
{
  const char * filename = "Wave.data";
  const char * names[] = {"y","z"};
  double t0 =  0.0;
//    const double t1 =  0.0;
  double t1 = 10.0;
  int    nt =  201;
//    const int    nt =  1;
//    const double dt = 0.0;
  const double dt = (t1 - t0)/(nt - 1);

  t0 += dt; t1 -= dt; nt = nt - 2;

  const double x0 = -5.0;
  const double x1 =  5.0;
  const int    nx =  101;
  const double dx = (x1 - x0)/(nx - 1);

  double * x = new double[nx];
  double * y = new double[nx];
  double * z = new double[nx];
  double * data[] = {y,z};
//    double * data[] = {y};

  for(int i=0; i<nx; i++) x[i] = x0 + i*dx;

  int status;

  {
    GIO_double1DC output(filename,names,2,status);
//      GIO_double1DC output(filename,names,1,status);
    if(!status) {
      cerr << "Opening output file failed" << endl;
      abort();
    }
    
    double Lb[1];
    double Ub[1];
    int ext[1];
    for(int n=0; n<nt; n++) {
      double t = t0 + n*dt;
      for(int i=0; i<nx; i++) {
	y[i] = wave(t,x[i]);
	z[i] = wave2(t,x[i]);
      }
      Lb[0] = x[0];
      Ub[0] = x[nx-1];
      ext[0] = nx;
      output.Write(t,Lb,Ub,ext,data,2);
//        output.Write(t,Lb,Ub,ext,data,1);
      //output.Write(t,ext,data,2);
    }
  }

  GIOquery query;
  if(!query.Read(filename)) {
    cerr << "Opening input file for query failed" << endl;
    abort();
  }
  query.Close();
  cout << "Query Key : " << query.Key() << endl;
  cout << "      Dim : " << query.Dim() << endl;

  {
    GIO_double1DC input(filename,status);
    if(!status) {
      cerr << "Opening input file failed" << endl;
      abort();
    }
    cout << "GIO Key = " << input.Key() << endl;
    cout << "Dimension = " << input.Dim() << endl;
    cout << "Ndatasets = " << input.NDataSets() << endl;
    cout << "Labels : ";
    for(int i=0; i<input.NDataSets(); i++) cout << input.Name(i) << " : ";
    cout << endl;
    cout << "Nseries = " << input.NSeries() << endl;
    for(int n=0; n < input.NSeries(); n++) {
      input.Read();
      cout << "I = " << n << " | Label = " << input.Label() << endl;
      for(int i=0; i<input.Dim(); i++) {
	cout << "[" << i << "] :: ";
	if(input.CoordsDefined())
	  cout << "LB(" << input.Lbound(0) << ") "
	       << "UB(" << input.Ubound(0) << ") ";
	cout << "ex(" << input.Ext(0)    << ")" << endl;
      }
    }
  }
  return 0;
}
