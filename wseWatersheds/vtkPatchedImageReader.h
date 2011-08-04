/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: vtkPatchedImageReader.h,v $
  Language:  C++
  Date:      $Date: 2005-12-21 20:59:57 $
  Version:   $Revision: 1.4 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPatchedImageReader.h,v $
  Language:  C++
  Date:      $Date: 2005-12-21 20:59:57 $
  Version:   $Revision: 1.4 $
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkPatchedImageReader - Superclass of binary file readers.
// .SECTION Description
// vtkPatchedImageReader provides methods needed to read a region from a file.
//
// THIS FILE IS MODIFIED FROM vtkImageReader.  IT IS IDENTICAL IN EVERY WAY TO
// vtkImageReader EXCEPT THAT IT EXTENDS SUPPORT TO MORE NATIVE DATA TYPES.
//
// .SECTION See Also
// vtkBMPReader vtkPNMReader vtkTIFFReader

#ifndef __vtkPatchedImageReader_h
#define __vtkPatchedImageReader_h

#include "vtkImageData.h"
#include "vtkImageSource.h"
#include "vtkTransform.h"

#ifndef vtkFloatingPointType
#define vtkFloatingPointType float
#endif

#define VTK_FILE_BYTE_ORDER_BIG_ENDIAN 0
#define VTK_FILE_BYTE_ORDER_LITTLE_ENDIAN 1

class VTK_EXPORT vtkPatchedImageReader : public vtkImageSource
{
public:
  static vtkPatchedImageReader *New();
  vtkTypeMacro(vtkPatchedImageReader,vtkImageSource);
  void PrintSelf(ostream& os, vtkIndent indent);   

  // Description:
  // Specify file name for the image file. You should specify either
  // a FileName or a FilePrefix. Use FilePrefix if the data is stored 
  // in multiple files.
  void SetFileName(const char *);
  vtkGetStringMacro(FileName);

  // Description:
  // Specify file prefix for the image file(s).You should specify either
  // a FileName or FilePrefix. Use FilePrefix if the data is stored
  // in multiple files.
  void SetFilePrefix(const char *);
  vtkGetStringMacro(FilePrefix);

  // Description:
  // The sprintf format used to build filename from FilePrefix and number.
  void SetFilePattern(const char *);
  vtkGetStringMacro(FilePattern);

  // Description:
  // When reading files which start at an unusual index, this can be added
  // to the slice number when generating the file name (default = 0)
  vtkSetMacro(FileNameSliceOffset,int);
  vtkGetMacro(FileNameSliceOffset,int);

  // Description:
  // When reading files which have regular, but non contiguous slices
  // (eg filename.1,filename.3,filename.5)
  // a spacing can be specified to skip missing files (default = 1)
  vtkSetMacro(FileNameSliceSpacing,int);
  vtkGetMacro(FileNameSliceSpacing,int);

  // Description:
  // Set the data type of pixels in the file.  
  // As a convenience, the OutputScalarType is set to the same value.
  // If you want the output scalar type to have a different value, set it
  // after this method is called.
  void SetDataScalarType(int type);
  void SetDataScalarTypeToFloat(){this->SetDataScalarType(VTK_FLOAT);}
  void SetDataScalarTypeToDouble(){this->SetDataScalarType(VTK_DOUBLE);}
  void SetDataScalarTypeToInt(){this->SetDataScalarType(VTK_INT);}
  void SetDataScalarTypeToShort(){this->SetDataScalarType(VTK_SHORT);}
  void SetDataScalarTypeToUnsignedLong(){this->SetDataScalarType(VTK_UNSIGNED_LONG);}
  void SetDataScalarTypeToUnsignedShort()
    {this->SetDataScalarType(VTK_UNSIGNED_SHORT);}
  void SetDataScalarTypeToUnsignedChar()
    {this->SetDataScalarType(VTK_UNSIGNED_CHAR);}

  // Description:
  // Return the maximum value in the output image. Valid only if the image is
  // of type unsigned long.
  unsigned long GetMaximumUnsignedLongValue();
  
  // Description:
  // Get the file format.  Pixels are this type in the file.
  vtkGetMacro(DataScalarType, int);

  // Description:
  // Set/Get the number of scalar components
  vtkSetMacro(NumberOfScalarComponents,int);
  vtkGetMacro(NumberOfScalarComponents,int);
  
  // Description:
  // Get/Set the extent of the data on disk.  
  vtkSetVector6Macro(DataExtent,int);
  vtkGetVector6Macro(DataExtent,int);
  
  // Description:
  // Set/get the data VOI. You can limit the reader to only
  // read a subset of the data. 
  vtkSetVector6Macro(DataVOI,int);
  vtkGetVector6Macro(DataVOI,int);
  
  // Description:
  // The number of dimensions stored in a file. This defaults to two.
  vtkSetMacro(FileDimensionality, int);
  int GetFileDimensionality() {return this->FileDimensionality;}
  
  // Description:
  // Set/Get the spacing of the data in the file.
  vtkSetVector3Macro(DataSpacing,double);
  vtkSetVector3Macro(DataSpacing,float);
  vtkGetVector3Macro(DataSpacing,double);
  
  // Description:
  // Set/Get the origin of the data (location of first pixel in the file).
  vtkSetVector3Macro(DataOrigin,double);
  vtkSetVector3Macro(DataOrigin,float);
  vtkGetVector3Macro(DataOrigin,double);

  // Description:
  // Get the size of the header computed by this object.
  unsigned long GetHeaderSize();
  unsigned long GetHeaderSize(int slice);

  // Description:
  // If there is a tail on the file, you want to explicitly set the
  // header size.
  void SetHeaderSize(unsigned long size);
  
  // Description:
  // Set/Get the Data mask.
  vtkGetMacro(DataMask,unsigned short);
  void SetDataMask(int val) 
       {if (val == this->DataMask) { return; }
        this->DataMask = ((unsigned short)(val)); this->Modified();}
  
  // Description:
  // Set/Get transformation matrix to transform the data from slice space
  // into world space. This matrix must be a permutation matrix. To qualify,
  // the sums of the rows must be + or - 1.
  vtkSetObjectMacro(Transform,vtkTransform);
  vtkGetObjectMacro(Transform,vtkTransform);

  // Description:
  // These methods should be used instead of the SwapBytes methods.
  // They indicate the byte ordering of the file you are trying
  // to read in. These methods will then either swap or not swap
  // the bytes depending on the byte ordering of the machine it is
  // being run on. For example, reading in a BigEndian file on a
  // BigEndian machine will result in no swapping. Trying to read
  // the same file on a LittleEndian machine will result in swapping.
  // As a quick note most UNIX machines are BigEndian while PC's
  // and VAX tend to be LittleEndian. So if the file you are reading
  // in was generated on a VAX or PC, SetDataByteOrderToLittleEndian 
  // otherwise SetDataByteOrderToBigEndian. 
  void SetDataByteOrderToBigEndian();
  void SetDataByteOrderToLittleEndian();
  int GetDataByteOrder();
  void SetDataByteOrder(int);
  const char *GetDataByteOrderAsString();

  // Description:
  // Set/Get the byte swapping to explicitly swap the bytes of a file.
  vtkSetMacro(SwapBytes,int);
  int GetSwapBytes() {return this->SwapBytes;}
  vtkBooleanMacro(SwapBytes,int);

//BTX
  ifstream *GetFile() {return this->File;}
  vtkGetVectorMacro(DataIncrements,unsigned long,4);
//ETX

  // Warning !!!
  // following should only be used by methods or template helpers, not users
  void ComputeInverseTransformedExtent(int inExtent[6],
                                       int outExtent[6]);
  void ComputeInverseTransformedIncrements(vtkIdType inIncr[3],
                                           vtkIdType outIncr[3]);

  void OpenFile();
  void OpenAndSeekFile(int extent[6], int slice);

  // Description:
  // Set/Get whether the data comes from the file starting in the lower left
  // corner or upper left corner.
  vtkBooleanMacro(FileLowerLeft, int);
  vtkGetMacro(FileLowerLeft, int);
  vtkSetMacro(FileLowerLeft, int);

  // Description:
  // Set/Get the internal file name
  void ComputeInternalFileName(int slice);
  vtkGetStringMacro(InternalFileName);
  
  
protected:
  vtkPatchedImageReader();
  ~vtkPatchedImageReader();

  char *InternalFileName;
  char *FileName;
  char *FilePrefix;
  char *FilePattern;
  int NumberOfScalarComponents;
  int FileLowerLeft;
  int FileNameSliceOffset;
  int FileNameSliceSpacing;

  ifstream *File;
  unsigned long DataIncrements[4];
  int DataExtent[6];
  unsigned short DataMask;  // Mask each pixel with ...
  int SwapBytes;

  int FileDimensionality;
  unsigned long HeaderSize;
  int DataScalarType;
  int ManualHeaderSize;
  int Initialized;
  vtkTransform *Transform;

  void ComputeTransformedSpacing (vtkFloatingPointType Spacing[3]);
  void ComputeTransformedOrigin (vtkFloatingPointType origin[3]);
  void ComputeTransformedExtent(int inExtent[6],
                                int outExtent[6]);
  void ComputeTransformedIncrements(int inIncr[3],
                                    int outIncr[3]);

  int DataDimensions[3];
  double DataSpacing[3];
  double DataOrigin[3];
  int DataVOI[6];
  
  void ExecuteInformation();
  void ExecuteData(vtkDataObject *data);
  virtual void ComputeDataIncrements();
private:
  vtkPatchedImageReader(const vtkPatchedImageReader&);  // Not implemented.
  void operator=(const vtkPatchedImageReader&);  // Not implemented.
};

#endif
