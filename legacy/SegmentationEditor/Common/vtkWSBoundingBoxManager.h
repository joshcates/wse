/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: vtkWSBoundingBoxManager.h,v $
  Language:  C++
  Date:      $Date: 2006-11-03 21:23:57 $
  Version:   $Revision: 1.4 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .Name vtkWSBoundingBox
// .Section Description
#ifndef __vtkWSBoundingBoxManager_
#define __vtkWSBoundingBoxManager_

#include "vtkWSBoundingBoxHash.h"
#include "vtkWSBoundingBox.h"
#include "vtkImageData.h"

class VTK_EXPORT vtkWSBoundingBoxManager : public vtkObject
{
public:
  static vtkWSBoundingBoxManager *New();

  vtkTypeMacro(vtkWSBoundingBoxManager,vtkObject);
  void PrintSelf(ostream&, vtkIndent) {}

  vtkSetObjectMacro(LabeledImage, vtkImageData);
  vtkGetObjectMacro(LabeledImage, vtkImageData);

  // Generates the hash table of bounding boxes from the LabeledImage data.
  void GenerateBoundingBoxes();

  bounding_box_t GetBoundingBox(unsigned long n);
  void GetBoundingBox(vtkWSBoundingBox *box, unsigned long n);       

  // Merges a list of boxes into a single box, returned as an array of 6 ints
  // [x0 x1 y0 y1 z0 z1].  Input is an array of unsigned long labels whose
  // first element denotes the number of labels to follow (i.e. length of the
  // array is list[0] + 1, where list[0] is not a label, but a size).
  //  bounding_box_t MergeBoundingBoxes(unsigned long *list);
  //  bounding_box_t MergeBoundingBoxesFromEquivalencyList(vtkWSLookupTableManager *manager);
    
protected:
  vtkWSBoundingBoxManager();
  ~vtkWSBoundingBoxManager() {}

private:
  vtkWSBoundingBoxHash BoundingBoxTable;
  vtkImageData *LabeledImage;
  
  
};

#endif
