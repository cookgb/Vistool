// -*- c++ -*-
//-------------------------------------------------------------------------
//
// $Id$
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//
// X11/Motif - C++ interface routines.
//
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>

#include <GL/GLwMDrawA.h>

#include "xvistool.h"

#define ANIMATETIME 20

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Installation routine for colormap if overlay menus used
void ensurePulldownColormapInstalled(Widget w, XtPointer client_data,
				     XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  XInstallColormap(mw->Xdisplay(), mw->XoverlayCM());
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void mw_openfs_cb(Widget, XtPointer, XtPointer);
void mw_file_cancel_cb(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Callback routine for main window File->Open menu selection
void mw_file_open(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;

  if(!mw->OpenDialog()) {
    mw->Open_Dialog = XmCreateFileSelectionDialog(mw->top_shell, "opendialog",
						  NULL, 0);
    XtAddCallback(mw->OpenDialog(), XmNokCallback, mw_openfs_cb, mw);
    XtAddCallback(mw->OpenDialog(), XmNcancelCallback, mw_file_cancel_cb, mw);
  }
  XtManageChild(mw->OpenDialog());
  XtPopup(XtParent(mw->OpenDialog()),XtGrabNone);
}

// Callback routine for main window File->Open->GIO menu selection
void mw_file_cancel_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  switch(mw->importtype) {
  case TYPE_GIO:
  case TYPE_1DDump:
    break;
  case TYPE_1DAb:
    mw->Abscissa_Set = !mw->Abscissa_Set;
    XmToggleButtonSetState(mw->CheckButton_1DAbs, mw->Abscissa_Set, False);
    break;
  }
  XtUnmanageChild(mw->OpenDialog());
}

// Callback routine for main window File->Open->GIO menu selection
void mw_file_open_GIO(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  mw->importtype = TYPE_GIO;
  mw_file_open(w, client_data, call_data);
}
// Callback routine for main window File->Open->GIO menu selection
void mw_file_open_1DDump(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  mw->importtype = TYPE_1DDump;
  mw_file_open(w, client_data, call_data);
}
// Callback routine for main window File->Open->GIO menu selection
void mw_file_open_1DDAb(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  mw->importtype = TYPE_1DAb;
  mw->Abscissa_Set = !mw->Abscissa_Set;
  if(mw->Abscissa_Set) {
    mw_file_open(w, client_data, call_data);
  } else {
    if(mw->Abscissa_Filename) delete [] mw->Abscissa_Filename;
    if(mw->Abscissa_Data) delete mw->Abscissa_Data;
    mw->Abscissa_Filename = 0;
    mw->Abscissa_Data = 0;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for open dialogue from main window File->Open 
void mw_openfs_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;

  XmFileSelectionBoxCallbackStruct * fs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  char * file;

  if(fs) {
    if(!XmStringGetLtoR(fs->value,XmFONTLIST_DEFAULT_TAG,&file))
      return; // internal error
    // Create and register the new drawwin
    XmString dirstr, patstr;
    XtVaGetValues(w,XmNdirectory,&dirstr,XmNpattern,&patstr,NULL);
    xvt_drawwin * dw = new xvt_drawwin(file,*mw,dirstr,patstr);
    switch(mw->importtype) {
    case TYPE_GIO:
      if(!dw->ImportFile_GIO(file)) dw->close();
      break;
    case TYPE_1DDump:
      if(!dw->ImportFile_1DDump(file)) dw->close();
      break;
    case TYPE_1DAb:
      if(dw->ImportFile_1DAbs(file)) {
	if(mw->Abscissa_Filename) delete [] mw->Abscissa_Filename;
	if(mw->Abscissa_Data) delete mw->Abscissa_Data;
	mw->Abscissa_Filename = dw->Abscissa_Filename;
	mw->Abscissa_Data = dw->Abscissa_Data;
	dw->Abscissa_Set = 0;
	dw->Abscissa_Filename = 0;
	dw->Abscissa_Data = 0;
      } else {
	mw->Abscissa_Set = !mw->Abscissa_Set;
	XmToggleButtonSetState(mw->CheckButton_1DAbs, mw->Abscissa_Set, False);
      }
      dw->close();
      break;
    }
    XtFree(file);
  }
  XtUnmanageChild(mw->OpenDialog());
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window File->Quit menu selection
void mw_file_quit(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  delete mw;
  exit(0);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window draw window list
void mw_windowlist(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  XmListCallbackStruct * cbs = (XmListCallbackStruct *) call_data;

  typedef std::list<vt_drawwin *>::iterator I_dw;
  I_dw p;
  for(p = mw->draw_list.begin(); p != mw->draw_list.end(); p++)
    (*p)->selected = false;
  mw->dw_listed = 0;

  const int sic = cbs->selected_item_count;

//    if(cbs->reason == XmCR_EXTENDED_SELECT) {
//      if(cbs->selection_type == XmINITIAL)
//        cout << "Extended selection -- initial selection: ";
//      else if(cbs->selection_type == XmMODIFICATION)
//        cout << "Extended selection -- modification of selection: ";
//      else if(cbs->selection_type == XmADDITION)
//        cout << "Extended selection -- additional selection: ";
//      cout << cbs->selected_item_count << " items selected" << endl;
    for(int i=0; i < sic; ++i) {
      const int sip = cbs->selected_item_positions[i];
      int k;
      for(p=mw->draw_list.begin(), k=1; p != mw->draw_list.end(); p++, k++)
	if(k == sip) {
	  (*p)->selected = true;
	  break;
	}

//        char * choice;
//        XmStringGetLtoR(cbs->selected_items[i], XmFONTLIST_DEFAULT_TAG,
//  		      &choice);
//        cout << (*p)->name << " | "
//  	   << choice << "(" << cbs->selected_item_positions[i] << ")" << endl;
//        XtFree(choice);
    }
    // Delete all items in dataset list and replace the list if only one
    // window is selected
    Widget sb;
    XtVaGetValues(mw->dataset_list,XmNhorizontalScrollBar,&sb,NULL);
    XtVaSetValues(sb,XmNvalue,0,NULL);
    XmListDeleteAllItems(mw->dataset_list);
    if(sic == 1) {
      vt_drawwin & dw = **p;
      mw->dw_listed = *p;
      typedef std::list<vt_data_series *>::iterator I_vd;
      I_vd d;
      for(d = dw.data_list.begin(); d != dw.data_list.end(); d++) {
	XmString DataName = XmStringCreateLocalized((String)(*d)->Name());
	XmListAddItemUnselected(mw->dataset_list,DataName,0);
	XmStringFree(DataName);
      }
      XtVaSetValues(sb,XmNvalue,0,NULL);
    }
    if(mw->ds_listed) { // erase the data set information
      mw->ds_listed = 0;
      XtVaSetValues(mw->info_label, XmNlabelString, mw->NULL_String, NULL);
    }
      
//    } else {
//      char * choice;
//      XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &choice);
//      cout << "Default action -- selcted item " << choice
//  	 << "(" << cbs->item_position << ")" << endl;
//        XtFree(choice);
//    }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window dataset list
void mw_datasetlist(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  XmListCallbackStruct * cbs = (XmListCallbackStruct *) call_data;

  vt_drawwin * dw = mw->dw_listed;
  if(!dw) return;

  typedef std::list<vt_data_series *>::iterator I_ds;
  I_ds p;
  for(p = dw->data_list.begin(); p != dw->data_list.end(); p++)
    (*p)->selected = false;
  mw->ds_listed = 0;

  const int sic = cbs->selected_item_count;

//    if(cbs->reason == XmCR_EXTENDED_SELECT) {
//      if(cbs->selection_type == XmINITIAL)
//        cout << "Extended selection -- initial selection: ";
//      else if(cbs->selection_type == XmMODIFICATION)
//        cout << "Extended selection -- modification of selection: ";
//      else if(cbs->selection_type == XmADDITION)
//        cout << "Extended selection -- additional selection: ";
//      cout << cbs->selected_item_count << " items selected" << endl;
    for(int i=0; i < sic; ++i) {
      const int sip = cbs->selected_item_positions[i];
      int k;
      for(p=dw->data_list.begin(), k=1; p != dw->data_list.end(); p++, k++)
	if(k == sip) {
	  (*p)->selected = true;
	  break;
	}

//        char * choice;
//        XmStringGetLtoR(cbs->selected_items[i], XmFONTLIST_DEFAULT_TAG,
//  		      &choice);
//        cout << (*p)->Name() << " | "
//  	   << choice << "(" << cbs->selected_item_positions[i] << ")" << endl;
//        XtFree(choice);
    }
    // Delete all items in dataset list and replace the list if only one
    // window is selected
    if(sic == 1) {
      vt_data_series & ds = **p;
      mw->ds_listed = *p;
      char * so = mw->Info_Name(ds.Origin());
      char * st = mw->Info_Time(ds.NSteps());
      char * sr = mw->Info_Range(ds.Bounds());
      const int len = 512;
      char * buf = new char[len];
      ostrstream info(buf,len);
      info << so << "\n" << st << "\n" << sr << ends;
      XmString InfoString = XmStringCreateLtoR(buf,"TAG1");
      XtVaSetValues(mw->info_label, XmNlabelString, InfoString, NULL);
      delete [] so; delete [] st; delete [] sr;
      delete [] buf;
      XmStringFree(InfoString);
    } else {
      mw->ds_listed = 0;
      XtVaSetValues(mw->info_label, XmNlabelString, mw->NULL_String, NULL);
    }
//    } else {
//      char * choice;
//      XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &choice);
//      cout << "Default action -- selcted item " << choice
//  	 << "(" << cbs->item_position << ")" << endl;
//        XtFree(choice);
//    }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window Animate->Syncronize menu slection
void mw_anim_sync(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  
  std::list<vt_drawwin *>::iterator p;
  int count = 0;
  for(p = mw->draw_list.begin(); p != mw->draw_list.end(); p++) {
    if((*p)->selected) ++count;
  }
  if(count <= 1) return; // do nothing unless 2 or more windows are selected.
  mw->sync_list.clear();
  for(p = mw->draw_list.begin(); p != mw->draw_list.end(); p++) {
    xvt_drawwin * dwp = (xvt_drawwin *) *p;
    dwp->sync_window = false;
    if(dwp->selected) {
      // Make sure animation gets turned off when a window is synced!
      if(dwp->animate) {
	mw->animateCount--;
	if(mw->animateCount == mw->animateHiddenCount)
	  XtRemoveTimeOut(mw->animateID);
	dwp->animate = false;
	dwp->SetAnimateToggle(false);
      }
      // And reset the lists.
      dwp->reset_list();
      // Finally, you can set the window to be synced.
      dwp->sync_window = true;
      mw->sync_list.push_back(dwp);
    }
  }
}

//----------------------------------------------------------------------------
// Callback routine for main window Animate->Un-Sync menu slection
void mw_anim_unsync(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_mainwin * mw = (xvt_mainwin *) client_data;
  
  std::list<vt_drawwin *>::iterator p;
  for(p = mw->sync_list.begin(); p != mw->sync_list.end(); p++) {
    (*p)->sync_window = false;
  }
  mw->sync_list.clear();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window Help->Help menu slection
void mw_help(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_mainwin * mw = (xvt_mainwin *) client_data;
  cout << "Selected Help" << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void handleAnimate(xvt_mainwin *, XtIntervalId *);
//----------------------------------------------------------------------------
// Installation routine for colormap if overlay menus used
void mapStateChanged(Widget w, XtPointer client_data, XEvent *event,
		     Boolean * cont)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  switch (event->type) {
  case MapNotify:
    if(dw->animate) {
      if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount)
	dw->xmvt.animateID =
	  XtAppAddTimeOut(dw->xmvt.Xapp(), ANIMATETIME,
			  (XtTimerCallbackProc) handleAnimate, &dw->xmvt);
      dw->xmvt.animateHiddenCount--;
    }
    dw->visible = true;
    break;
  case UnmapNotify:
    if(dw->animate) {
      dw->xmvt.animateHiddenCount++;
      if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount) {
	XtRemoveTimeOut(dw->xmvt.animateID);
      }
    }
    dw->visible = false;
    break;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_LockFullFrame(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  dw->fullframe = !dw->fullframe;
  if(!dw->fullframe) dw->reset_CurrentBounds();
  dw->postRedisplay();
}

void dw_ZoomCurrent(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  if(dw->fullframe) {
    dw->SetFullZoomToggle(false);
    dw->reset_CurrentBounds();
  }
  dw->push_CurrentBounds(dw->Minimum_CurrentBounds());
  dw->postRedisplay();
}

void dw_ZoomReset(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  if(dw->fullframe) dw->SetFullZoomToggle(false);
  dw->reset_CurrentBounds();
  dw->postRedisplay();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_UnZoom(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  if(dw->fullframe) return;
  dw->pop_CurrentBounds();
  dw->postRedisplay();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void handleAnimate(xvt_mainwin * xmvt, XtIntervalId * id)
{
  xmvt->incrementAnimation();
  xmvt->animateID = XtAppAddTimeOut(xmvt->Xapp(), ANIMATETIME,
				    (XtTimerCallbackProc) handleAnimate, xmvt);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_animate(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  dw->animate = !dw->animate;
  if(dw->animate) {
    if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount)
      dw->xmvt.animateID =
	XtAppAddTimeOut(dw->xmvt.Xapp(), ANIMATETIME,
			(XtTimerCallbackProc) handleAnimate, &dw->xmvt);
    dw->xmvt.animateCount++;
  } else {
    dw->xmvt.animateCount--;
    if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount)
      XtRemoveTimeOut(dw->xmvt.animateID);
  }
  if(!dw->xmvt.sync_dup && dw->sync_window) { // repeat on any synced windows
    dw->xmvt.sync_dup = true;
    std::list<vt_drawwin *>::iterator p;
    for(p = dw->xmvt.sync_list.begin(); p != dw->xmvt.sync_list.end(); p++) {
      xvt_drawwin * dwp = (xvt_drawwin *) *p;
      if(dw != dwp) {
	dw_animate(w,dwp,call_data);
	dwp->SetAnimateToggle(dw->animate);
      }
    }
    dw->xmvt.sync_dup = false;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_stepforward(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  if(dw->animate) {
    dw->xmvt.animateCount--;
    if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount)
      XtRemoveTimeOut(dw->xmvt.animateID);
    dw->animate = false;
    dw->SetAnimateToggle(false);
    //
    dw->xmvt.sync_dup = true;
    if(dw->sync_window) {
      std::list<vt_drawwin *>::iterator p;
      for(p = dw->xmvt.sync_list.begin(); p != dw->xmvt.sync_list.end(); p++) {
	xvt_drawwin * dwp = (xvt_drawwin *) *p;
	if(dw != dwp)
	  dw_stepforward(w,dwp,call_data);
      }
    }
    dw->xmvt.sync_dup = false;
  }
  if(!dw->xmvt.sync_dup) {
    dw->increment();
    dw->postRedisplay();
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_stepbackward(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  if(dw->animate) {
    dw->xmvt.animateCount--;
    if(dw->xmvt.animateCount == dw->xmvt.animateHiddenCount)
      XtRemoveTimeOut(dw->xmvt.animateID);
    dw->animate = false;
    dw->SetAnimateToggle(false);
    //
    dw->xmvt.sync_dup = true;
    if(dw->sync_window) {
      std::list<vt_drawwin *>::iterator p;
      for(p = dw->xmvt.sync_list.begin(); p != dw->xmvt.sync_list.end(); p++) {
	xvt_drawwin * dwp = (xvt_drawwin *) *p;
	if(dw != dwp)
	  dw_stepbackward(w,dwp,call_data);
      }
    }
    dw->xmvt.sync_dup = false;
  }
  if(!dw->xmvt.sync_dup) {
    dw->decrement();
    dw->postRedisplay();
  }
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_reset_animate(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  dw->reset_list();
  dw->postRedisplay();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for main window File->Close menu selection
void dw_file_close(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  if(dw) {
    dw->close();
  } else {
    cerr << "client data not set in drawwin close." << endl;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void dw_openfs_cb(Widget, XtPointer, XtPointer);
void dw_file_cancel_cb(Widget, XtPointer, XtPointer);
//----------------------------------------------------------------------------
// Callback routine for drawing window File->Open menu selection
void dw_file_open(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  if(!dw->OpenDialog()) {
    Arg dirpat[2];
    XtSetArg(dirpat[0],XmNdirectory,dw->search_dir);
    XtSetArg(dirpat[1],XmNpattern,dw->search_pattern);
    dw->Open_Dialog = XmCreateFileSelectionDialog(dw->draw_shell, "opendialog",
						  dirpat, 2);
    XtAddCallback(dw->OpenDialog(), XmNokCallback, dw_openfs_cb, dw);
    XtAddCallback(dw->OpenDialog(), XmNcancelCallback, dw_file_cancel_cb, dw);
    XmStringFree(dw->search_dir);
    XmStringFree(dw->search_pattern);
  }
  XtManageChild(dw->OpenDialog());
  XtPopup(XtParent(dw->OpenDialog()),XtGrabNone);
}

// Callback routine for main window File->Open->GIO menu selection
void dw_file_cancel_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  switch(dw->importtype) {
  case TYPE_GIO:
  case TYPE_1DDump:
    break;
  case TYPE_1DAb:
    dw->Abscissa_Set = !dw->Abscissa_Set;
    XmToggleButtonSetState(dw->CheckButton_1DAbs, dw->Abscissa_Set, False);
    break;
  }
  XtUnmanageChild(dw->OpenDialog());
}
// Callback routine for drawing window File->Open menu selection
void dw_file_open_GIO(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  dw->importtype = TYPE_GIO;
  dw_file_open(w, client_data, call_data);
}
// Callback routine for drawing window File->Open menu selection
void dw_file_open_1DDump(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  dw->importtype = TYPE_1DDump;
  dw_file_open(w, client_data, call_data);
}
// Callback routine for drawing window File->Open menu selection
void dw_file_open_1DDAb(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;
  dw->importtype = TYPE_1DAb;
  dw->Abscissa_Set = !dw->Abscissa_Set;
  if(dw->Abscissa_Set) {
    dw_file_open(w, client_data, call_data);
  } else {
    if(dw->Abscissa_Filename) delete [] dw->Abscissa_Filename;
    if(dw->Abscissa_Data) delete dw->Abscissa_Data;
    dw->Abscissa_Filename = 0;
    dw->Abscissa_Data = 0;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for open dialogue from main window File->Open 
void dw_openfs_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  xvt_drawwin * dw = (xvt_drawwin *) client_data;

  XmFileSelectionBoxCallbackStruct * fs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  char * file;

  if(fs) {
    if(!XmStringGetLtoR(fs->value,XmFONTLIST_DEFAULT_TAG,&file))
      return; // internal error
    switch(dw->importtype) {
    case TYPE_GIO:
      if(dw->ImportFile_GIO(file)) {
	dw->postRedisplay();
      }
      break;
    case TYPE_1DDump:
      if(dw->ImportFile_1DDump(file)) {
	dw->postRedisplay();
      }
      break;
    case TYPE_1DAb:
      if(!(dw->ImportFile_1DAbs(file))) {
	dw->Abscissa_Set = !dw->Abscissa_Set;
	XmToggleButtonSetState(w, dw->Abscissa_Set, False);
	if(dw->Abscissa_Filename) delete [] dw->Abscissa_Filename;
	if(dw->Abscissa_Data) delete dw->Abscissa_Data;
      }
      break;
    }
    XtFree(file);
    if((dw->xmvt).dw_listed == dw) { // Redisplay data set list
      vt_data_series * ds = dw->data_list.back();
      XmString DataName = XmStringCreateLocalized((String)ds->Name());
      XmListAddItemUnselected((dw->xmvt).dataset_list,DataName,0);
      XmStringFree(DataName);
    }
  }
  XtUnmanageChild(dw->OpenDialog());
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Callback routine for drawing window Help->Help menu slection
void dw_help(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_drawwin * dw = (xvt_drawwin *) client_data;
  cout << "Selected Help in a draw window" << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void activatePopup(Widget w, XtPointer client_data, XEvent *event,
		   Boolean * cont)
{
  Widget popup = (Widget) client_data;
  XButtonPressedEvent *bevent = (XButtonPressedEvent *) event;
  if(bevent->button != 3) return;
  XmMenuPosition(popup, bevent);
  XtManageChild(popup);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_popup_reset(Widget w, XtPointer client_data, XtPointer call_data)
{
//    xvt_drawwin * dw = (xvt_drawwin *) client_data;
  cout << "Reset popup button selected." << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button1DownAction(Widget w, XEvent * event, String * params, 
		       Cardinal * num_params)
{
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL); // Get the xvt_drawwin.
  XButtonEvent * bevent = (XButtonEvent *) event;
  dw->xpin_RB = dw->xorg_RB = bevent->x;
  dw->ypin_RB = dw->yorg_RB = bevent->y;
  dw->xwid_RB = dw->ywid_RB = 0;
  glXWaitGL();
  dw->DrawRubberBand();
  dw->draw_RB = true;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button1MotionAction(Widget w, XEvent * event, String * params, 
			 Cardinal * num_params)
{
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL); // Get the xvt_drawwin.
  XButtonEvent * bevent = (XButtonEvent *) event;
  glXWaitGL();
  dw->DrawRubberBand();
  (bevent->x < dw->xpin_RB) ? dw->xorg_RB=bevent->x : dw->xorg_RB=dw->xpin_RB;
  (bevent->y < dw->ypin_RB) ? dw->yorg_RB=bevent->y : dw->yorg_RB=dw->ypin_RB;
  dw->xwid_RB = abs(bevent->x - dw->xpin_RB);
  dw->ywid_RB = abs(bevent->y - dw->ypin_RB);
  dw->DrawRubberBand();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button1UpAction(Widget w, XEvent * event, String * params, 
		     Cardinal * num_params)
{
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL); // Get the xvt_drawwin.
  glXWaitGL();
  dw->DrawRubberBand();
  double xl;
  double xu;
  double yl;
  double yu;
  const double x_int   = dw->Cur_Bounds->Lb_x;
  const double x_slope = (dw->Cur_Bounds->Ub_x - x_int)/dw->cur_width;
  const double y_int   = dw->Cur_Bounds->Ub_y;
  const double y_slope = (dw->Cur_Bounds->Lb_y - y_int)/dw->cur_height;
  if(dw->xwid_RB != 0 || dw->ywid_RB != 0) {
    xl = dw->xorg_RB*x_slope + x_int;
    xu = (dw->xorg_RB + dw->xwid_RB + 1)*x_slope + x_int;
    yu = dw->yorg_RB*y_slope + y_int;
    yl = (dw->yorg_RB + dw->ywid_RB + 1)*y_slope + y_int;
  } else {
    const double dx = 0.125*(dw->Cur_Bounds->Ub_x - x_int);
    const double dy = 0.125*(y_int - dw->Cur_Bounds->Lb_y);
    xl = (dw->xorg_RB + 0.5)*x_slope + x_int;
    xu = xl + dx;
    xl -= dx;
    yu = (dw->yorg_RB + 0.5)*y_slope + y_int;
    yl = yu - dy;
    yu += dy;
  }
  dw->draw_RB = false;
  dw->xpin_RB = dw->ypin_RB = 0;
  dw->xorg_RB = dw->yorg_RB = 0;
  dw->xwid_RB = dw->ywid_RB = 0;
  dw->push_CurrentBounds(new bounds_2D(xl,yl,xu,yu));
  dw->Coords_Text(true);
  dw->postRedisplay();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button2DownAction(Widget w, XEvent * event, String * params, 
		       Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 2 Down: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button2MotionAction(Widget w, XEvent * event, String * params, 
			 Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 2 Motion: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void Button2UpAction(Widget w, XEvent * event, String * params, 
		     Cardinal * num_params)
{
  // Get the xvt_drawwin.
  xvt_drawwin * dw = NULL;
  XtVaGetValues(w,XmNuserData,&dw,NULL);
  //if(!dw) {cerr << "Error getting xvt_drawwin pointer" << endl; abort();}

//    const int x = event->xbutton.x;
//    const int y = event->xbutton.y;

//    cout << "Button 2 Up: \n"
//         << "Mouse at (" << x << "," << y << ")\n"
//         << "String : " << params << "\n"
//         << "Number of params : " << *num_params << endl;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_draw(Widget w, XtPointer data, XtPointer callData)
{
  xvt_drawwin * dw = (xvt_drawwin *) data;

  dw->postRedisplay();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_resize(Widget w, XtPointer data, XtPointer callData)
{
  xvt_drawwin * dw = (xvt_drawwin *) data;
  GLwDrawingAreaCallbackStruct * resize =
    (GLwDrawingAreaCallbackStruct *) callData;

  glXWaitX(); // Wait until X events are processed.
  dw->resize(resize->width,resize->height);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void dw_init(Widget w, XtPointer data, XtPointer callData)
{
  xvt_drawwin * dw = (xvt_drawwin *) data;
  
  dw->init(dw->wWidth(), dw->wHeight());
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 
void handleRedisplay(xvt_mainwin * xmvt)
{
  xmvt->redisplay();
}






