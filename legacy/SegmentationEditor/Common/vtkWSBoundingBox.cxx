/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: vtkWSBoundingBox.cxx,v $
  Language:  C++
  Date:      $Date: 2006-11-03 21:23:57 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkObjectFactory.h"
#include "vtkWSBoundingBox.h"

vtkWSBoundingBox* vtkWSBoundingBox::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkWSBoundingBox");
  if(ret)
    {
    return (vtkWSBoundingBox*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkWSBoundingBox;
}

vtkWSBoundingBox::vtkWSBoundingBox()
{
  Extent[0] = 0; Extent[1] = 0;
  Extent[2] = 0; Extent[3] = 0;
  Extent[4] = 0; Extent[5] = 0;
}

void vtkWSBoundingBox::SetExtent(int x0, int x1, int y0, int y1, int z0, int z1)
{
  Extent[0] = x0; Extent[1] = x1;
  Extent[2] = y0; Extent[3] = y1;
  Extent[4] = z0; Extent[5] = z1;
}

void vtkWSBoundingBox::Merge(const vtkWSBoundingBox *b)
{
  if ( b->Extent[0] < Extent[0] ) Extent[0] = b->Extent[0];
  if ( b->Extent[2] < Extent[2] ) Extent[2] = b->Extent[2];
  if ( b->Extent[4] < Extent[4] ) Extent[4] = b->Extent[4];
  
  if ( b->Extent[1] > Extent[1] ) Extent[1] = b->Extent[1];
  if ( b->Extent[3] > Extent[3] ) Extent[3] = b->Extent[3];
  if ( b->Extent[5] > Extent[5] ) Extent[5] = b->Extent[5];
}
