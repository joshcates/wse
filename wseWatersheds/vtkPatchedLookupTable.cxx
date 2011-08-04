/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: vtkPatchedLookupTable.cxx,v $
  Language:  C++
  Date:      $Date: 2003-05-01 23:16:41 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPatchedLookupTable.cxx,v $
  Language:  C++
  Date:      $Date: 2003-05-01 23:16:41 $
  Version:   $Revision: 1.1 $


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
#include <math.h>
#include "vtkPatchedLookupTable.h"
#include "vtkLookupTableEquivalencyHash.h"
#include "vtkBitArray.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkPatchedLookupTable* vtkPatchedLookupTable::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPatchedLookupTable");
  if(ret)
    {
    return (vtkPatchedLookupTable*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPatchedLookupTable;
}

// Construct with range=(0,1); and hsv ranges set up for rainbow color table 
// (from red to blue).
vtkPatchedLookupTable::vtkPatchedLookupTable(unsigned long sze,
                                                        unsigned long ext)
{
  this->NumberOfColors = sze;
  this->Table = vtkUnsignedCharArray::New();
  this->Table->SetNumberOfComponents(4);
  this->Table->Allocate(4*sze,4*ext);

  this->HueRange[0] = 0.0;
  this->HueRange[1] = 0.66667;

  this->SaturationRange[0] = 1.0;
  this->SaturationRange[1] = 1.0;

  this->ValueRange[0] = 1.0;
  this->ValueRange[1] = 1.0;

  this->AlphaRange[0] = 1.0;
  this->AlphaRange[1] = 1.0;
  this->Alpha = 1.0;

  this->TableRange[0] = 0.0;
  this->TableRange[1] = 1.0;

  this->Ramp = VTK_RAMP_SCURVE;
  this->Scale = VTK_SCALE_LINEAR;
}

vtkPatchedLookupTable::~vtkPatchedLookupTable()
{
  this->Table->Delete();
  this->Table = NULL;
}

// Scalar values greater than maximum range value are clamped to maximum
// range value.
void vtkPatchedLookupTable::SetTableRange(double r[2])
{
  this->SetTableRange(r[0],r[1]);
}

// Set the minimum/maximum scalar values for scalar mapping. Scalar values
// less than minimum range value are clamped to minimum range value.
// Scalar values greater than maximum range value are clamped to maximum
// range value.
void vtkPatchedLookupTable::SetTableRange(double rmin, double rmax)
{
  if (this->Scale == VTK_SCALE_LOG10 && 
      ((rmin > 0 && rmax < 0) || (rmin < 0 && rmax > 0)))
    {
    vtkErrorMacro("Bad table range for log scale: ["<<rmin<<", "<<rmax<<"]");
    return;
    }
  if (rmax < rmin)
    {
    vtkErrorMacro("Bad table range: ["<<rmin<<", "<<rmax<<"]");
    return;
    }

  if (this->TableRange[0] == rmin && this->TableRange[1] == rmax)
    {
    return;
    }

  this->TableRange[0] = rmin;
  this->TableRange[1] = rmax;

  this->Modified();
}

// Have to be careful about the range if scale is logarithmic
void vtkPatchedLookupTable::SetScale(int scale)
{
  if (this->Scale == scale)
    {
    return;
    }
  this->Scale = scale;
  this->Modified();

  double rmin = this->TableRange[0];
  double rmax = this->TableRange[1];

  if (this->Scale == VTK_SCALE_LOG10 && 
      ((rmin > 0 && rmax < 0) || (rmin < 0 && rmax > 0)))
    {
    this->TableRange[0] = 1.0f;
    this->TableRange[1] = 10.0f;
    vtkErrorMacro("Bad table range for log scale: ["<<rmin<<", "<<rmax<<"], "
                  "adjusting to [1, 10]");
    return;
    }
}

// Allocate a color table of specified size.
int vtkPatchedLookupTable::Allocate(unsigned long sz, unsigned long ext)
{
  this->NumberOfColors = sz;
  int a = this->Table->Allocate(4*this->NumberOfColors,4*ext);
  this->Modified();
  return a;
}

// Generate lookup table from hue, saturation, value, alpha min/max values. 
// Table is built from linear ramp of each value.
void vtkPatchedLookupTable::Build()
{
  int hueCase;
  double hue, sat, val, lx, ly, lz, frac, hinc, sinc, vinc, ainc;
  double rgba[4], alpha;
  unsigned char *c_rgba;
  unsigned long i;
  unsigned long maxIndex = this->NumberOfColors - 1;

  if (this->Table->GetNumberOfTuples() < 1 ||
      (this->GetMTime() > this->BuildTime && 
       this->InsertTime < this->BuildTime))
    {
    hinc = (this->HueRange[1] - this->HueRange[0])/maxIndex;
    sinc = (this->SaturationRange[1] - this->SaturationRange[0])/maxIndex;
    vinc = (this->ValueRange[1] - this->ValueRange[0])/maxIndex;
    ainc = (this->AlphaRange[1] - this->AlphaRange[0])/maxIndex;

    for (i = 0; i <= maxIndex; i++) 
      {
      hue = this->HueRange[0] + i*hinc;
      sat = this->SaturationRange[0] + i*sinc;
      val = this->ValueRange[0] + i*vinc;
      alpha = this->AlphaRange[0] + i*ainc;

      hueCase = static_cast<int>(hue * 6);
      frac = 6*hue - hueCase;
      lx = val*(1.0 - sat);
      ly = val*(1.0 - sat*frac);
      lz = val*(1.0 - sat*(1.0 - frac));

      switch (hueCase) 
      {
        /* 0<hue<1/6 */
      case 0:
      case 6:
        rgba[0] = val;
        rgba[1] = lz;
        rgba[2] = lx;
        break;
        /* 1/6<hue<2/6 */
      case 1:
        rgba[0] = ly;
        rgba[1] = val;
        rgba[2] = lx;
        break;
        /* 2/6<hue<3/6 */
      case 2:
        rgba[0] = lx;
        rgba[1] = val;
        rgba[2] = lz;
        break;
        /* 3/6<hue/4/6 */
      case 3:
        rgba[0] = lx;
        rgba[1] = ly;
        rgba[2] = val;
        break;
        /* 4/6<hue<5/6 */
      case 4:
        rgba[0] = lz;
        rgba[1] = lx;
        rgba[2] = val;
        break;
        /* 5/6<hue<1 */
      case 5:
        rgba[0] = val;
        rgba[1] = lx;
        rgba[2] = ly;
        break;
      }
      rgba[3] = alpha;

      c_rgba = this->Table->WritePointer(4*i,4);

      if (this->Ramp == VTK_RAMP_SCURVE)
        {
        c_rgba[0] = static_cast<unsigned char> 
          (127.5*(1.0+cos((1.0-static_cast<double>(rgba[0]))*3.141593)));
        c_rgba[1] = static_cast<unsigned char> 
          (127.5*(1.0+cos((1.0-static_cast<double>(rgba[1]))*3.141593)));
        c_rgba[2] = static_cast<unsigned char> 
          (127.5*(1.0+cos((1.0-static_cast<double>(rgba[2]))*3.141593)));
        c_rgba[3] = static_cast<unsigned char> (alpha*255.0);
        /* same code, but with rounding 
        c_rgba[0] = static_cast<unsigned char> 
          (127.5f*(1.0f + (double)cos(double((1.0f-rgba[0])*3.141593f)))+0.5f);
        c_rgba[1] = static_cast<unsigned char> 
          (127.5f*(1.0f + (double)cos(double((1.0f-rgba[1])*3.141593f)))+0.5f);
        c_rgba[2] = static_cast<unsigned char> 
          (127.5f*(1.0f + (double)cos(double((1.0f-rgba[2])*3.141593f)))+0.5f);
        c_rgba[3] = static_cast<unsigned char>(rgba[3]*255.0f + 0.5f);
        */
        }
      else
        {
        c_rgba[0] = static_cast<unsigned char>(rgba[0]*255.0f + 0.5f);
        c_rgba[1] = static_cast<unsigned char>(rgba[1]*255.0f + 0.5f);
        c_rgba[2] = static_cast<unsigned char>(rgba[2]*255.0f + 0.5f);
        c_rgba[3] = static_cast<unsigned char>(rgba[3]*255.0f + 0.5f);
        }
    }
    this->BuildTime.Modified();
  }
}

// get the color for a scalar value
void vtkPatchedLookupTable::GetColor(double v, double rgb[3])
{
  unsigned char *rgb8 = this->MapValue(v);

  rgb[0] = rgb8[0]/255.0;
  rgb[1] = rgb8[1]/255.0;
  rgb[2] = rgb8[2]/255.0;
}

// get the opacity (alpha) for a scalar value
double vtkPatchedLookupTable::GetOpacity(double v)
{
  unsigned char *rgb8 = this->MapValue(v);

  return rgb8[3]/255.0;
}

// There is a little more to this than simply taking the log10 of the
// two range values: we do conversion of negative ranges to positive
// ranges, and conversion of zero to a 'very small number'
void vtkPatchedLookupTableLogRange(double range[2], double logRange[2])
{
  double rmin = range[0];
  double rmax = range[1];

  if (rmin == 0)
    {
    rmin = 1.0e-6*(rmax - rmin);
    if (rmax < 0)
      {
      rmin = -rmin;
      }
    }
  if (rmax == 0)
    {
    rmax = 1.0e-6*(rmin - rmax);
    if (rmin < 0)
      {
      rmax = -rmax;
      }
    }
  if (rmin < 0 && rmax < 0)
    {
    logRange[0] = log10(-(double)rmin);
    logRange[1] = log10(-(double)rmax);
    }
  else if (rmin > 0 && rmax > 0)
    {
    logRange[0] = log10((double)rmin);
    logRange[1] = log10((double)rmax);
    }
}

// Apply log to value, with appropriate constraints.
static inline double vtkApplyLogScale(double v, double range[2], 
                                     double logRange[2])
{
  // is the range set for negative numbers?
  if (range[0] < 0)
    {
    if (v < 0)
      {
      v = log10(-static_cast<double>(v));
      }
    else if (range[0] > range[1])
      {
      v = logRange[0];
      }
    else
      {
      v = logRange[1];
      }
    }
  else
    {
    if (v > 0)
      {
      v = log10(static_cast<double>(v));
      }
    else if (range[0] < range[1])
      {
      v = logRange[0];
      }
    else
      {
      v = logRange[1];
      }
    }
  return v;
}                 

// Apply shift/scale to the scalar value v and do table lookup.
static inline unsigned char *vtkLinearLookup(double v,   
                                             unsigned char *table,
                                             double maxIndex,
                                             double shift, double scale)
{
  double findx = (v + shift)*scale;
  if (findx < 0)
    {
    findx = 0;
    }
  if (findx > maxIndex)
    {
    findx = maxIndex;
    }
  return &table[4*static_cast<int>(findx)];
  /* round
  return &table[4*(int)(findx + 0.5f)];
  */
}

// Given a scalar value v, return an rgba color value from lookup table.
unsigned char *vtkPatchedLookupTable::MapValue(double v)
{
  double maxIndex = this->NumberOfColors - 1;
  double shift, scale;

  if (this->Scale == VTK_SCALE_LOG10)
    {   // handle logarithmic scale
    double logRange[2];
    vtkPatchedLookupTableLogRange(this->TableRange, logRange);
    shift = -logRange[0];
    if (logRange[1] <= logRange[0])
      {
      scale = VTK_LARGE_FLOAT;
      }
    else
      {
      scale = (maxIndex + 1)/(logRange[1] - logRange[0]);
      }
    /* correct scale
    scale = maxIndex/(logRange[1] - logRange[0]);
    */
    v = vtkApplyLogScale(v, this->TableRange, logRange);
    }
  else
    {   // plain old linear
    shift = -this->TableRange[0];
    if (this->TableRange[1] <= this->TableRange[0])
      {
      scale = VTK_LARGE_FLOAT;
      }
    else
      {
      scale = (maxIndex + 1)/(this->TableRange[1] - this->TableRange[0]);
      }
    /* correct scale
    scale = maxIndex/(this->TableRange[1] - this->TableRange[0]);
    */
    }

  // this is the same for log or linear
  return vtkLinearLookup(v, this->Table->GetPointer(0), maxIndex, 
                         shift, scale); 
}

// accelerate the mapping by copying the data in 32-bit chunks instead
// of 8-bit chunks
template<class T>
static void vtkPatchedLookupTableMapData(vtkPatchedLookupTable *self, T *input, 
                                  unsigned char *output, int length, 
                                  int inIncr, int outFormat)
{
  int i = length;
  double *range = self->GetTableRange();
  double maxIndex = self->GetNumberOfColors() - 1;
  double shift, scale;
  unsigned char *table = self->GetPointer(0);
  unsigned char *cptr;
  double alpha;

  if ( (alpha=self->GetAlpha()) >= 1.0 ) //no blending required 
    {
    if (self->GetScale() == VTK_SCALE_LOG10)
      {
      double val;
      double logRange[2];
      vtkPatchedLookupTableLogRange(range, logRange);
      shift = -logRange[0];
      if (logRange[1] <= logRange[0])
        {
        scale = VTK_LARGE_FLOAT;
        }
      else
        {
        scale = (maxIndex + 1)/(logRange[1] - logRange[0]);
        }
      /* correct scale
      scale = maxIndex/(logRange[1] - logRange[0]);
      */
      if (outFormat == VTK_RGBA)
        {
        while (--i >= 0) 
          {
          val = vtkApplyLogScale(*input, range, logRange);
          cptr = vtkLinearLookup(val, table, maxIndex, shift, scale); 
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;     
          input += inIncr;
          }
        }
      else if (outFormat == VTK_RGB)
        {
        while (--i >= 0) 
          {
          val = vtkApplyLogScale(*input, range, logRange);
          cptr = vtkLinearLookup(val, table, maxIndex, shift, scale); 
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;
          input += inIncr;
          }
        }
      else if (outFormat == VTK_LUMINANCE_ALPHA)
        {
        while (--i >= 0) 
          {
          val = vtkApplyLogScale(*input, range, logRange);
          cptr = vtkLinearLookup(val, table, maxIndex, shift, scale); 
          *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 + 
                                                 cptr[2]*0.11 + 0.5);
          *output++ = cptr[3];
          input += inIncr;
          }
        }
      else // outFormat == VTK_LUMINANCE
        {
        while (--i >= 0) 
          {
          val = vtkApplyLogScale(*input, range, logRange);
          cptr = vtkLinearLookup(val, table, maxIndex, shift, scale); 
          *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 + 
                                                 cptr[2]*0.11 + 0.5);
          input += inIncr;
          }
        }
      }//if log scale

    else //not log scale
      {
      shift = -range[0];
      if (range[1] <= range[0])
        {
        scale = VTK_LARGE_FLOAT;
        }
      else
        {
        scale = (maxIndex + 1)/(range[1] - range[0]);
        }
      /* correct scale
      scale = maxIndex/(range[1] - range[0]);
      */

      if (outFormat == VTK_RGBA)
        {
        while (--i >= 0) 
          {
          cptr = vtkLinearLookup(*input, table, maxIndex, shift, scale); 
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;     
          input += inIncr;
          }
        }
      else if (outFormat == VTK_RGB)
        {
        while (--i >= 0) 
          {
          cptr = vtkLinearLookup(*input, table, maxIndex, shift, scale); 
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;
          input += inIncr;
          }
        }
      else if (outFormat == VTK_LUMINANCE_ALPHA)
        {
        while (--i >= 0) 
          {
          cptr = vtkLinearLookup(*input, table, maxIndex, shift, scale); 
          *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 + 
                                                 cptr[2]*0.11 + 0.5);
          *output++ = cptr[3];
          input += inIncr;
          }
        }
      else // outFormat == VTK_LUMINANCE
        {
        while (--i >= 0) 
          {
          cptr = vtkLinearLookup(*input, table, maxIndex, shift, scale); 
          *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 + 
                                                 cptr[2]*0.11 + 0.5);
          input += inIncr;
          }
        }
      }//if not log lookup
    }//if blending not needed

  else //blend with the specified alpha
    {
    if (self->GetScale() == VTK_SCALE_LOG10)
      {
      double val;
      double logRange[2];
      vtkPatchedLookupTableLogRange(range, logRange);
      shift = -logRange[0];
      if (logRange[1] <= logRange[0])
        {
        scale = VTK_LARGE_FLOAT;
        }
      else
        {
        scale = (maxIndex + 1)/(logRange[1] - logRange[0]);
        }
      /* correct scale
      scale = maxIndex/(logRange[1] - logRange[0]);
      */
      if (outFormat == VTK_RGBA)
        {
        while (--i >= 0) 
          {
          val = vtkApplyLogScale(*input, range, logRange);
          cptr = vtkLinearLookup(val, table, maxIndex, shift, scale); 
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = static_cast<unsigned char>((*cptr)*alpha); cptr++;
          input += inIncr;
          }
        }
      else if (outFormat == VTK_RGB)
        {
        while (--i >= 0) 
          {
          val = vtkApplyLogScale(*input, range, logRange);
          cptr = vtkLinearLookup(val, table, maxIndex, shift, scale); 
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;
          input += inIncr;
          }
        }
      else if (outFormat == VTK_LUMINANCE_ALPHA)
        {
        while (--i >= 0) 
          {
          val = vtkApplyLogScale(*input, range, logRange);
          cptr = vtkLinearLookup(val, table, maxIndex, shift, scale); 
          *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 + 
                                                 cptr[2]*0.11 + 0.5);
          *output++ = static_cast<unsigned char>(alpha*cptr[3]);
          input += inIncr;
          }
        }
      else // outFormat == VTK_LUMINANCE
        {
        while (--i >= 0) 
          {
          val = vtkApplyLogScale(*input, range, logRange);
          cptr = vtkLinearLookup(val, table, maxIndex, shift, scale); 
          *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 + 
                                                 cptr[2]*0.11 + 0.5);
          input += inIncr;
          }
        }
      }//log scale with blending

    else //no log scale with blending
      {
      shift = -range[0];
      if (range[1] <= range[0])
        {
        scale = VTK_LARGE_FLOAT;
        }
      else
        {
        scale = (maxIndex + 1)/(range[1] - range[0]);
        }
      /* correct scale
      scale = maxIndex/(range[1] - range[0]);
      */

      if (outFormat == VTK_RGBA)
        {
        while (--i >= 0) 
          {
          cptr = vtkLinearLookup(*input, table, maxIndex, shift, scale); 
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = static_cast<unsigned char>((*cptr)*alpha); cptr++;
          input += inIncr;
          }
        }
      else if (outFormat == VTK_RGB)
        {
        while (--i >= 0) 
          {
          cptr = vtkLinearLookup(*input, table, maxIndex, shift, scale); 
          *output++ = *cptr++;
          *output++ = *cptr++;
          *output++ = *cptr++;
          input += inIncr;
          }
        }
      else if (outFormat == VTK_LUMINANCE_ALPHA)
        {
        while (--i >= 0) 
          {
          cptr = vtkLinearLookup(*input, table, maxIndex, shift, scale); 
          *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 + 
                                                 cptr[2]*0.11 + 0.5);
          *output++ = static_cast<unsigned char>(cptr[3]*alpha);
          input += inIncr;
          }
        }
      else // outFormat == VTK_LUMINANCE
        {
        while (--i >= 0) 
          {
          cptr = vtkLinearLookup(*input, table, maxIndex, shift, scale); 
          *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 + 
                                                 cptr[2]*0.11 + 0.5);
          input += inIncr;
          }
        }
      }//no log scale
    }//alpha blending
}

void vtkPatchedLookupTable::MapScalarsThroughTable2(void *input, 
                                             unsigned char *output,
                                             int inputDataType, 
                                             int numberOfValues,
                                             int inputIncrement,
                                             int outputFormat)
{
  switch (inputDataType)
    {
    case VTK_BIT:
      {
      vtkIdType i, id;
      vtkBitArray *bitArray = vtkBitArray::New();
      bitArray->SetVoidArray(input,numberOfValues,1);
      vtkUnsignedCharArray *newInput = vtkUnsignedCharArray::New();
      newInput->SetNumberOfValues(numberOfValues);
      for (id=i=0; i<numberOfValues; i++, id+=inputIncrement)
        {
        newInput->SetValue(i, bitArray->GetValue(id));
        }
      vtkPatchedLookupTableMapData(this,
                            static_cast<unsigned char*>(newInput->GetPointer(0)),
                            output,numberOfValues,
                            inputIncrement,outputFormat);
      newInput->Delete();
      bitArray->Delete();
      }
      break;
      
    case VTK_CHAR:
      vtkPatchedLookupTableMapData(this,static_cast<char *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    case VTK_UNSIGNED_CHAR:
      vtkPatchedLookupTableMapData(this,static_cast<unsigned char *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    case VTK_SHORT:
      vtkPatchedLookupTableMapData(this,static_cast<short *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
    break;
      
    case VTK_UNSIGNED_SHORT:
      vtkPatchedLookupTableMapData(this,static_cast<unsigned short *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    case VTK_INT:
      vtkPatchedLookupTableMapData(this,static_cast<int *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    case VTK_UNSIGNED_INT:
      vtkPatchedLookupTableMapData(this,static_cast<unsigned int *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    case VTK_LONG:
      vtkPatchedLookupTableMapData(this,static_cast<long *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    case VTK_UNSIGNED_LONG:
      vtkPatchedLookupTableMapData(this,static_cast<unsigned long *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    case VTK_FLOAT:
      vtkPatchedLookupTableMapData(this,static_cast<float *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    case VTK_DOUBLE:
      vtkPatchedLookupTableMapData(this,static_cast<double *>(input),output,
                            numberOfValues,inputIncrement,outputFormat);
      break;
      
    default:
      vtkErrorMacro(<< "MapImageThroughTable: Unknown input ScalarType");
      return;
    }
}  

// Specify the number of values (i.e., colors) in the lookup
// table. This method simply allocates memory and prepares the table
// for use with SetTableValue(). It differs from Build() method in
// that the allocated memory is not initialized according to HSVA ramps.
void vtkPatchedLookupTable::SetNumberOfTableValues(unsigned long number)
{
  this->NumberOfColors = number;
  this->Table->SetNumberOfTuples(number);
}

// Directly load color into lookup table. Use [0,1] float values for color
// component specification. Make sure that you've either used the
// Build() method or used SetNumberOfTableValues() prior to using this method.
void vtkPatchedLookupTable::SetTableValue(unsigned long indx, double rgba[4])
{
  unsigned char *_rgba = this->Table->WritePointer(4*indx,4);

  _rgba[0] = static_cast<unsigned char>(rgba[0]*255.0f + 0.5f);
  _rgba[1] = static_cast<unsigned char>(rgba[1]*255.0f + 0.5f);
  _rgba[2] = static_cast<unsigned char>(rgba[2]*255.0f + 0.5f);
  _rgba[3] = static_cast<unsigned char>(rgba[3]*255.0f + 0.5f);

  this->InsertTime.Modified();
  this->Modified();
}

// Directly load color into lookup table. Use [0,1] double values for color 
// component specification.
void vtkPatchedLookupTable::SetTableValue(unsigned long indx, double
                                                     r, double g, double b,
                                                     double a)
{
  double rgba[4];
  rgba[0] = r; rgba[1] = g; rgba[2] = b; rgba[3] = a;
  this->SetTableValue(indx,rgba);
}

// Return a rgba color value for the given index into the lookup Table. Color
// components are expressed as [0,1] double values.
void vtkPatchedLookupTable::GetTableValue(unsigned long indx, double rgba[4])
{
  unsigned char *_rgba;

  if (indx > this->NumberOfColors) indx = this->NumberOfColors - 1;
  
  _rgba = this->Table->GetPointer(indx*4);

  rgba[0] = _rgba[0]/255.0;
  rgba[1] = _rgba[1]/255.0;
  rgba[2] = _rgba[2]/255.0;
  rgba[3] = _rgba[3]/255.0;
}

// Return a rgba color value for the given index into the lookup table. Color
// components are expressed as [0,1] double values.
double *vtkPatchedLookupTable::GetTableValue(unsigned long indx)
{
  this->GetTableValue(indx, this->RGBA);
  return this->RGBA;
}

void vtkPatchedLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkScalarsToColors::PrintSelf(os,indent);

  os << indent << "TableRange: (" << this->TableRange[0] << ", "
     << this->TableRange[1] << ")\n";
  os << indent << "Scale: " 
     << (this->Scale == VTK_SCALE_LOG10 ? "Log10\n" : "Linear\n");    
  os << indent << "HueRange: (" << this->HueRange[0] << ", "
     << this->HueRange[1] << ")\n";
  os << indent << "SaturationRange: (" << this->SaturationRange[0] << ", "
     << this->SaturationRange[1] << ")\n";
  os << indent << "ValueRange: (" << this->ValueRange[0] << ", "
     << this->ValueRange[1] << ")\n";
  os << indent << "AlphaRange: (" << this->AlphaRange[0] << ", "
     << this->AlphaRange[1] << ")\n";
  os << indent << "NumberOfTableValues: " 
     << this->GetNumberOfTableValues() << "\n";
  os << indent << "NumberOfColors: " << this->NumberOfColors << "\n";
  os << indent << "Ramp: "
     << (this->Ramp == VTK_RAMP_SCURVE ? "SCurve\n" : "Linear\n");  
  os << indent << "InsertTime: " <<this->InsertTime.GetMTime() << "\n";
  os << indent << "BuildTime: " <<this->BuildTime.GetMTime() << "\n";
}


