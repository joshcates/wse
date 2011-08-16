/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: vtkWSBoundingBoxHash.h,v $
  Language:  C++
  Date:      $Date: 2006-11-03 21:23:57 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __vtkWSBoundingBoxHash_h_
#define __vtkWSBoundingBoxHash_h_

#include "itk_hash_map.h"

struct bounding_box_t
{
  int x0;
  int x1;
  int y0;
  int y1;
  int z0;
  int z1;
};

typedef itk::hash_map<unsigned long, bounding_box_t, itk::hash<unsigned long> >
  vtkWSBoundingBoxHash;


#endif
