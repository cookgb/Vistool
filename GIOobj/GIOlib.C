// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include "GIOlib.h"
#include "GIOtempl.c"

#ifdef SPX
#pragma define(GIOseries<  float,  float, 1,GIO_FLOAT_SERIES>)
#pragma define(GIOseries<  float,  float, 2,GIO_FLOAT_SERIES>)
#pragma define(GIOseries<  float,  float, 3,GIO_FLOAT_SERIES>)
#pragma define(GIOcseries< float,  float, 1,GIO_FLOAT_CSERIES>)
#pragma define(GIOcseries< float,  float, 2,GIO_FLOAT_CSERIES>)
#pragma define(GIOcseries< float,  float, 3,GIO_FLOAT_CSERIES>)
#pragma define(GIOseries< double, double, 1,GIO_DOUBLE_SERIES>)
#pragma define(GIOseries< double, double, 2,GIO_DOUBLE_SERIES>)
#pragma define(GIOseries< double, double, 3,GIO_DOUBLE_SERIES>)
#pragma define(GIOcseries< double, double, 1,GIO_DOUBLE_CSERIES>)
#pragma define(GIOcseries< double, double, 2,GIO_DOUBLE_CSERIES>)
#pragma define(GIOcseries< double, double, 3,GIO_DOUBLE_CSERIES>)
#else
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
#endif
