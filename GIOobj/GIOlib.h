// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#ifndef GIOLIB_H
#define GIOLIB_H

#include <fstream>
#include <iostream>
#include "GIOincl.h"

enum GIOtype {
  GIO_FLOAT_SERIES=0,
  GIO_FLOAT_CSERIES=1,
  GIO_DOUBLE_SERIES = 2,
  GIO_DOUBLE_CSERIES = 3
};

typedef GIOseries<  float, float,1,GIO_FLOAT_SERIES>   GIO_float1D;
typedef GIOseries<  float, float,2,GIO_FLOAT_SERIES>   GIO_float2D;
typedef GIOseries<  float, float,3,GIO_FLOAT_SERIES>   GIO_float3D;
typedef GIOcseries< float, float,1,GIO_FLOAT_CSERIES>  GIO_float1DC;
typedef GIOcseries< float, float,2,GIO_FLOAT_CSERIES>  GIO_float2DC;
typedef GIOcseries< float, float,3,GIO_FLOAT_CSERIES>  GIO_float3DC;
typedef GIOseries< double,double,1,GIO_DOUBLE_SERIES>  GIO_double1D;
typedef GIOseries< double,double,2,GIO_DOUBLE_SERIES>  GIO_double2D;
typedef GIOseries< double,double,3,GIO_DOUBLE_SERIES>  GIO_double3D;
typedef GIOcseries<double,double,1,GIO_DOUBLE_CSERIES> GIO_double1DC;
typedef GIOcseries<double,double,2,GIO_DOUBLE_CSERIES> GIO_double2DC;
typedef GIOcseries<double,double,3,GIO_DOUBLE_CSERIES> GIO_double3DC;

#endif // GIOLIB_H
