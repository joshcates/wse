/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: vtkBinaryVolume.h,v $
  Language:  C++
  Date:      $Date: 2003-09-11 20:36:43 $
  Version:   $Revision: 1.3 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// .Name vtkBinaryVolume
// .Section Description
// 
#ifndef __vtkBinaryVolume_
#define __vtkBinaryVolume_

#include "vtkImageData.h"

class VTK_EXPORT vtkBinaryVolume : public vtkImageData
{
public:
  static vtkBinaryVolume *New();

  vtkTypeMacro(vtkBinaryVolume,vtkImageData);
  void PrintSelf(ostream&, vtkIndent) {}

  void Clear();

  void SetWithRadius(int x, int y, int z);
  void UnsetWithRadius(int x, int y, int z);

  void SetLabelValue(int v)
  { this->m_LabelValue = (unsigned char) v; }
  void SetLabelValue(unsigned char v)
  { this->m_LabelValue = v; }
  
  void  Set(int x, int y, int z)
  {    *( (unsigned char *)( GetScalarPointer(x, y, z) )) = m_LabelValue;}
  void Unset(int x, int y, int z)
    { *( (unsigned char *)( GetScalarPointer(x, y, z) )) = 0;}
  bool   Get(int x, int y, int z)
    { return (*( (unsigned char *)( GetScalarPointer(x, y, z) )) != 0); }

  int WriteToDisk(const char *fn);
  int ReadFromDisk(const char *fn);
  
  // For interfacing with Tcl.
  int   GetAsInt(int x, int y, int z)
    { return (int) (*( (unsigned char *)( GetScalarPointer(x, y, z) ))); }
  float GetAsFloat(int x, int y, int z)
    { return (float) (*( (unsigned char *)( GetScalarPointer(x, y, z) ))); }

  void SetPaintRadius(int r)
    { paint_radius = r; }

  int GetPaintRadius()
    { return paint_radius;}
  
protected:
  int paint_radius;

  unsigned char m_LabelValue;
  
  vtkBinaryVolume();
  ~vtkBinaryVolume() {}
  
private:

  
};

#endif
