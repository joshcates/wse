//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#ifndef IMAGE_H
#define IMAGE_H

#include "itkImage.h"
#include <QtGui>
#include "vtkITKUtility.h"
#include "itkVTKImageExport.h"
#include "vtkImageImport.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkDiscreteGaussianImageFilter.h"

/** */
class Image
{
public:
  typedef itk::Image<float,3> itkFloatImage;
  typedef itk::LinearInterpolateImageFunction<itkFloatImage, double >  LinearInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction<itkFloatImage, double >  NearestNeighborInterpolatorType;

  Image();
  ~Image();

  Image(itkFloatImage *img);

  //  Image &operator=(itkFloatImage *img)
  // { 
  //  mOriginal = img; 
  //  return *this;
  // }

  bool load(QString fname);

  // temporarily not used--this is the bit depth information
  int depth()
  {
    return 1;
    //    if (mOriginal) return mOriginal->depth;
    // else return 0;
  }
  int width()
  {
    if (mOriginal) return mOriginal->GetBufferedRegion().GetSize()[0];
    else return 0;
  }
  int height()
  {
    if (mOriginal) return mOriginal->GetBufferedRegion().GetSize()[1];
    else return 0;
  }
  int nSlices()
  {
    if (mOriginal)
    {
      if (mOriginal->GetImageDimension() > 2)
      {
        return mOriginal->GetBufferedRegion().GetSize()[2];
      }
      else return 1;
    }
    else return 0;
    
  }
  // temporarily only 1 channel
  int nChannels()    
  {
    return 1;
    //  if (mOriginal) return mOriginal->nChannels; else return 0;
  }
  

  bool isBinarySegmentation();

  itkFloatImage::PixelType getPixel(int i, int j, int k);
  itkFloatImage::PixelType getPixel(int ijk[3]);

  itkFloatImage::PixelType getPhysicalPixel(float x, float y, float z);
  
  itkFloatImage::PixelType getLinearInterpolatedPixel(double point[3]);
  itkFloatImage::PixelType getNearestInterpolatedPixel(double point[3]);


  itkFloatImage::PixelType getGaussianPixel(double point[3]);


  int getSliceForPoint(double p[3]);
  double getClosestSlicePoint(double point[3]);


  itkFloatImage::Pointer original() { return mOriginal; }
  //  itkFloatImage::Pointer modified() { return mModified; }
  
  const vtkImageImport *originalVTK() const { return mVTKImportOriginal; }
  //  const vtkImageImport *modifiedVTK() const { return mVTKImportModified; }
  vtkImageImport *originalVTK() { return mVTKImportOriginal;}
  //  vtkImageImport *modifiedVTK() { return mVTKImportModified;}
  //  vtkImageImport *resampledVTK(SmoothSettings settings);
  
  QColor color() const { return mColor; } 
  QString name() const { return mName; } 
  void name(const QString &n)  { mName = n; }
  
  LinearInterpolatorType::Pointer mLinearInterpolator;
  NearestNeighborInterpolatorType::Pointer mNearestInterpolator;
  LinearInterpolatorType::Pointer mGaussianInterpolator;

  float getMinimumSpacing();

private:

  //  void generateResampledImage(SmoothSettings settings);
  void generateGaussian();

  itkFloatImage::Pointer mOriginal;
  //  itkFloatImage::Pointer mModified;
  itkFloatImage::RegionType mOriginalRegion;
  QColor mColor;
  QString mName;
  
  /** These members can act as VTK pipeline sources.  vtkImageImport is
      connected to itk::VTKImageExport. */
  vtkImageImport * mVTKImportOriginal;
  //  vtkImageImport * mVTKImportModified;
  vtkImageImport * mVTKImportResampled;
  itk::VTKImageExport<itkFloatImage>::Pointer mITKExporterOriginal;
  //  itk::VTKImageExport<itkFloatImage>::Pointer mITKExporterModified;
  itk::VTKImageExport<itkFloatImage>::Pointer mITKExporterResampled;

  //  SmoothSettings lastSmoothSettings;

  itk::DiscreteGaussianImageFilter<itkFloatImage, itkFloatImage>::Pointer mImageGaussian;
};





#endif
