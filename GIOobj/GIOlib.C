// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include "GIOlib.h"
#include "GIOtempl.c"

template class  GIOseries<  float,  float, 1,GIO_FLOAT_SERIES>;
template class  GIOseries<  float,  float, 2,GIO_FLOAT_SERIES>;
template class  GIOseries<  float,  float, 3,GIO_FLOAT_SERIES>;
template class  GIOcseries< float,  float, 1,GIO_FLOAT_CSERIES>;
template class  GIOcseries< float,  float, 2,GIO_FLOAT_CSERIES>;
template class  GIOcseries< float,  float, 3,GIO_FLOAT_CSERIES>;
template class  GIOseries< double, double, 1,GIO_DOUBLE_SERIES>;
template class  GIOseries< double, double, 2,GIO_DOUBLE_SERIES>;
template class  GIOseries< double, double, 3,GIO_DOUBLE_SERIES>;
template class  GIOcseries< double, double, 1,GIO_DOUBLE_CSERIES>;
template class  GIOcseries< double, double, 2,GIO_DOUBLE_CSERIES>;
template class  GIOcseries< double, double, 3,GIO_DOUBLE_CSERIES>;
