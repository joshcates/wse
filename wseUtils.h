//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------

#ifndef UTILS_H
#define UTILS_H

#include <QtGui>
//#include <cv.h>


class vtkImageData;

namespace wse {

//QImage IplImageToQImage(const IplImage *iplImage);
bool QImageToVTKImage(const QImage &img, vtkImageData *vtkimage);
void QColorToVTKColor(const QColor &qc, double *c);


#ifdef WIN32
void RedirectIOToConsole();
#endif

} // end namespace


#endif
