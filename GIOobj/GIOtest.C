
#include <iostream.h>
#include <math.h>

#include "GIO1d.h"


double wave(double t, double x)
{
  return 1.5*sin(t - x);
}

main()
{
  const char * filename = "Wave.data";
  const char * names[] = {"x","y"};
  const double t0 =  0.0;
  const double t1 = 10.0;
  const int    nt =  201;
  const double dt = (t1 - t0)/(nt - 1);
  const double x0 = -5.0;
  const double x1 =  5.0;
  const int    nx =  101;
  const double dx = (x1 - x0)/(nx - 1);

  double * x = new double[nx];
  double * y = new double[nx];
  double * data[] = {x,y};

  for(int i=0; i<nx; i++) x[i] = x0 + i*dx;

  bool status;

  {
    GIO1dseries<double,double> output(filename,names,2,status);
    if(!status) {
      cerr << "Opening output file failed" << endl;
      abort();
    }
    
    for(int n=0; n<nt; n++) {
      double t = t0 + n*dt;
      for(int i=1; i<nx; i++) y[i] = wave(t,x[i]);
      output.Write(t,nx,data,2);
    }
  }

  {
    GIO1dseries<double,double> input(filename,status);
    if(!status) {
      cerr << "Opening input file failed" << endl;
      abort();
    }
    cout << "Dimension = " << input.Dim() << endl;
    cout << "Ndatasets = " << input.NDataSets() << endl;
    cout << "Labels : ";
    for(int i=0; i<input.NDataSets(); i++) cout << input.Name(i) << " : ";
    cout << endl;
    cout << "Nseries = " << input.NSeries() << endl;
    for(int n=0; n < input.NSeries(); n++) {
      input.Read();
      cout << "I = " << n << " | Label = " << input.Label() << endl;
      cout << "Extent = " << input.Ext(0) << endl;
    }
  }
}
