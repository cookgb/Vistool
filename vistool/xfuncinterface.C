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
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  vt_drawwin * ndw = new xvt_drawwin(*dw);
  ndw->Apply(Abs);
}

void apply_log(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  vt_drawwin * ndw = new xvt_drawwin(*dw);
  ndw->Apply(Log);
}

void apply_ln(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  vt_drawwin * ndw = new xvt_drawwin(*dw);
  ndw->Apply(Ln);
}

void apply_dt(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  vt_drawwin * ndw = new xvt_drawwin(*dw);
  ndw->Apply_Seq(T_Deriv);
}

void norm_Linf(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  dw->Norm(Linfinity);
}

void norm_L1(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  dw->Norm(L1);
}

void norm_L2(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  dw->Norm(L2);
}
