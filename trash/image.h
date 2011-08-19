#ifndef _wse_image_h
#define _wse_image_h

#include "itkImage.h"
#include <iostream>

// Qt Includes
#include <QtGui>

// ITK Includes
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// VTK Includes
#include "vtkImageData.h"
#include "itkVTKImageExport.h"
#include "vtkImageImport.h"
#include "vtkITKUtility.h"

namespace wse {

/** A wrapper for itk::Image that provides a number of convenient
    functions, including loading and saving from disk and
    interpolation of subpixel image values.  This class also provides
    convenient export to VTK images through a single method. */
class Image
{
 public:
  typedef itk::Image<float,3> itkFloatImage;
  typedef itk::LinearInterpolateImageFunction<itkFloatImage, double >  LinearInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction<itkFloatImage, double >  
    NearestNeighborInterpolatorType;
  
 Image() : mITKImage(NULL), mName(""), mVTKImport(NULL)
    {  
      mColor = QColor(100,100,100);
      //  mColor = QColor(cvRandInt(&rng)%255,cvRandInt(&rng)%255,cvRandInt(&rng)%255);
    }
  ~Image()  { if (mVTKImport != NULL) mVTKImport->Delete();   }
  
  /** Construct an image using an itk::ImagePointer */
  Image(itkFloatImage *img);

  /** Loads an image from the file "fname" using the
      itk::ImageFileReader. */
  bool read(QString fname);

  /** Writes an image to the file "fname" using the
      itk::ImageFileWriter. */
  bool write(QString fname) const;

  // temporarily not used--this is the bit depth information
  //  int depth()
  //  {    return 1;  }
  
  /** Initializes vtk Importer/Exporter objects and hooks up the
      ITK->VTK conversion pipeline. Intended to be called only once on
      initialization. */
  void constructVTKPipeline();

  /** Inializes the ITK image interpolator objects and hooks them up
      to the itk image.  Intended to be called only once on
      initialization. */
  void constructImageInterpolators();
    
  /** Returns the number of pixels on the x-axis of the image */
  inline int x() const { return this->width(); }
  int width() const
  {
    if (mITKImage) return mITKImage->GetBufferedRegion().GetSize()[0];
    else return 0;
  }
  
  /** Returns the number of pixels on the y-axis of the image */
  inline int y() const { return this->height(); }
  int height() const
  {
    if (mITKImage) return mITKImage->GetBufferedRegion().GetSize()[1];
    else return 0;
  }

  /** Returns the number of pixels on the z-axis of the image */
  inline int z() const { return this->nSlices(); }
  int nSlices() const
  {
    if (mITKImage)
    {
      if (mITKImage->GetImageDimension() > 2)
	{ return mITKImage->GetBufferedRegion().GetSize()[2]; }
      else 
	{ return 1; }
    }
    else return 0;
    
  }

  /** Returns the number of channels in the image. Temporarily only supports 1 channel.*/
  //  int nChannels()    
  // {
  //  return 1;
    //  if (mITKImage) return mITKImage->nChannels; else return 0;
  // }
  

  /** Returns true if this image has a range of only 2 values. */
  bool isBinarySegmentation();

  /** Returns the pixel value at the (i,j,k) location */
  itkFloatImage::PixelType getPixel(int i, int j, int k) const;
  itkFloatImage::PixelType getPixel(int ijk[3]) const;

  /** Returns the pixel value at the (x,y,z) "physical" coordinate location. */
  //  itkFloatImage::PixelType getPhysicalPixel(float x, float y, float z);
  
  /** Returns the interpolated (linear) pixel value at the given (x,y,z) point. */
  itkFloatImage::PixelType getLinearInterpolatedPixel(double point[3]) const;

  /** Returns the interpolated (nearest-neighbor) pixel value at the given (x,y,z) point. */
  itkFloatImage::PixelType getNearestInterpolatedPixel(double point[3]) const;

  /** Returns the slice number that contains the given point. */
  int getSliceForPoint(double p[3]) const;

  /** Returns the closest slice number to the given point */
  double getClosestSlicePoint(double point[3]) const;

  /** Returns the pointer to the itkImage. */
  itkFloatImage::Pointer itkImage() { return mITKImage; }
  itkFloatImage::ConstPointer itkImage() const 
    {
      return itkFloatImage::ConstPointer(mITKImage); 
    }

  /** */
  float getMinimumSpacing() const;

  /** */  
  const vtkImageImport *vtkImporter() const { return mVTKImport; }
  vtkImageImport *vtkImporter() { return mVTKImport;}
  
  /** Return the color that has been associated with this image
      (useful for GUIs).*/
  QColor color() const { return mColor; } 

  /** Set a color to associated with this image (useful for GUIs). */
  void color(const QColor &c) { mColor = c; }

  /** Return the name that has been associated with this image (useful
      for GUIs). Note that this is not a file name. */
  QString name() const { return mName; } 

  /** Set a name for this image (useful for GUIs).  Note that this is
      not a file name.*/
  void name(const QString &n)  { mName = n; }
  
private:
  /** Linear interpolator used by getPixel functions. */
  LinearInterpolatorType::Pointer mLinearInterpolator;

  /** Nearest-neighbor interpolator used by getPixl functions. */
  NearestNeighborInterpolatorType::Pointer mNearestInterpolator;

  /** This is the itk::SmartPointer to the actual ITK image data. */
  itkFloatImage::Pointer mITKImage;

  /** A color that represents this image.  It could be used in GUI icons, for example. */
  QColor mColor;

  /** A name for this image. */
  QString mName;
  
  /** These members can act as VTK pipeline sources.  vtkImageImport is
      connected to itk::VTKImageExport. */
  vtkImageImport * mVTKImport;
  itk::VTKImageExport<itkFloatImage>::Pointer mITKExporter;
};

} // end namespace wse
#endif
