// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <iostream>
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
      std::cerr << "Opening output file failed" << std::endl;
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
    std::cerr << "Opening input file for query failed" << std::endl;
    abort();
  }
  query.Close();
  std::cout << "Query Key : " << query.Key() << std::endl;
  std::cout << "      Dim : " << query.Dim() << std::endl;

  {
    GIO_double1DC input(filename,status);
    if(!status) {
      std::cerr << "Opening input file failed" << std::endl;
      abort();
    }
    std::cout << "GIO Key = " << input.Key() << std::endl;
    std::cout << "Dimension = " << input.Dim() << std::endl;
    std::cout << "Ndatasets = " << input.NDataSets() << std::endl;
    std::cout << "Labels : ";
    for(int i=0; i<input.NDataSets(); i++) std::cout << input.Name(i) << " : ";
    std::cout << std::endl;
    std::cout << "Nseries = " << input.NSeries() << std::endl;
    for(int n=0; n < input.NSeries(); n++) {
      input.Read();
      std::cout << "I = " << n << " | Label = " << input.Label() << std::endl;
      for(int i=0; i<input.Dim(); i++) {
	std::cout << "[" << i << "] :: ";
	if(input.CoordsDefined())
	  std::cout << "LB(" << input.Lbound(0) << ") "
	       << "UB(" << input.Ubound(0) << ") ";
	std::cout << "ex(" << input.Ext(0)    << ")" << std::endl;
      }
    }
  }
  return 0;
}

