
#include <iostream.h>

#include "GIO1d.h"

main()
{
  const char * filename = "GIOtest.data";
  {
    const char * names[] = {"x","y"};
    bool status;
    GIO1dseries<double,double> output(filename,names,2,status);
    cout << "Write status is : " << status << endl;
    double x[] = {1.,2.,3.,4.,5.};
    double y[] = {6.,7.,8.,9.,10.};
    double * data[] = {x,y};
    output.Write(0.0,5,data,2);
  }
  {
    bool status;
    GIO1dseries<double,double> input(filename,status);
    cout << "Read status is : " << status << endl;
    status = input.Read();
    cout << "Read status is : " << status << endl;
  }
    
}
