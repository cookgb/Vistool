// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//
// X11 interface to functions
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include "xvistool.h"

void apply_copy(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  new xvt_drawwin(*dw);
}

void apply_abs(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_drawwin * dw = (xvt_drawwin *) client_data;

  cout << "applying abs..." << endl;
}

void apply_log(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_drawwin * dw = (xvt_drawwin *) client_data;

  cout << "applying log..." << endl;
}

void apply_ln(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_drawwin * dw = (xvt_drawwin *) client_data;

  cout << "applying ln..." << endl;
}
