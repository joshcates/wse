/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkWatershedSegmentTreeWriter.txx,v $
  Language:  C++
  Date:      $Date: 2003-05-01 23:18:26 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkWatershedSegmentTreeWriter_txx
#define __itkWatershedSegmentTreeWriter_txx

#include "itkWatershedSegmentTreeWriter.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

namespace itk
{

template <class TScalarType>
void WatershedSegmentTreeWriter<TScalarType>
::Write()
{
  const unsigned BUFSZ = 16384;

  typename SegmentTreeType::Pointer input = this->GetInput();

  std::ofstream out(m_FileName.c_str(), std::ios::binary);
  //std::ofstream out("/scratch/data/tumorbase/case1/spgr/tree.tree", std::ios::binary);

  if (!out)
    {
      throw ExceptionObject(__FILE__, __LINE__);
    }

  // write header
  unsigned long listsz = input->Size();

  out.write((char *)&listsz, sizeof(unsigned long));
  
  // now write data
  typename SegmentTreeType::ValueType *buf =
    new typename SegmentTreeType::ValueType[BUFSZ];
  
  typename SegmentTreeType::Iterator it;

  it = input->Begin();

  unsigned n = 0;
  while ( it != input->End() )
    {
      buf[n] = *it;
      n++;
      ++it;
      if (n == BUFSZ)
        {
          out.write((char *)buf,
                    sizeof (typename SegmentTreeType::ValueType) *  BUFSZ);
          n = 0;
        }
    }
  out.write((char *)buf,
            sizeof (typename SegmentTreeType::ValueType) *  n);

  out.close();
  delete[] buf; 
}

} // end namespace itk

#endif

