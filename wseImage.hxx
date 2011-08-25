#ifndef _wse_image_hxx
#define _wse_image_hxx

#include <iostream>
#include <limits>


// Qt Includes
#include <QtGui>

// ITK Includes
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionConstIterator.h"

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
template<class T>
class Image
{
 public:
  typedef itk::Image<T,3> itkImageType;
  typedef itk::LinearInterpolateImageFunction<itkImageType, double >  LinearInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction<itkImageType, double >  
    NearestNeighborInterpolatorType;
  
 Image() : mITKImage(NULL), mName(""), mVTKImport(NULL)
    {  
      mColor = QColor(100,100,100);
      //  mColor = QColor(cvRandInt(&rng)%255,cvRandInt(&rng)%255,cvRandInt(&rng)%255);
    }
  ~Image()  { if (mVTKImport != NULL) mVTKImport->Delete();   }
  
  /** Construct an image using an itk::ImagePointer */
  Image(itkImageType *img);

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
  inline unsigned int x() const { return this->width(); }
  unsigned int width() const
  {
    if (mITKImage) return mITKImage->GetBufferedRegion().GetSize()[0];
    else return 0;
  }
  
  /** Returns the number of pixels on the y-axis of the image */
  inline unsigned int y() const { return this->height(); }
  unsigned int height() const
  {
    if (mITKImage) return mITKImage->GetBufferedRegion().GetSize()[1];
    else return 0;
  }

  /** Returns the number of pixels on the z-axis of the image */
  inline unsigned int z() const { return this->nSlices(); }
  unsigned int nSlices() const
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
  typename itkImageType::PixelType getPixel(int i, int j, int k) const;
  typename itkImageType::PixelType getPixel(int ijk[3]) const;

  /** Returns the pixel value at the (x,y,z) "physical" coordinate location. */
  //  itkImageType::PixelType getPhysicalPixel(T x, T y, T z);
  
  /** Returns the interpolated (linear) pixel value at the given (x,y,z) point. */
  typename itkImageType::PixelType getLinearInterpolatedPixel(double point[3]) const;

  /** Returns the interpolated (nearest-neighbor) pixel value at the given (x,y,z) point. */
  typename itkImageType::PixelType getNearestInterpolatedPixel(double point[3]) const;

  /** Returns the slice number that contains the given point. */
  int getSliceForPoint(double p[3]) const;

  /** Returns the closest slice number to the given point */
  double getClosestSlicePoint(double point[3]) const;

  /** Returns the pointer to the itkImage. */
  typename itkImageType::Pointer itkImage() { return mITKImage; }
  typename itkImageType::ConstPointer itkImage() const 
    {
      return typename itkImageType::ConstPointer(mITKImage); 
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

  /** Computes the maximum value stored in the image.  Note that this
      requires a traversal of the image each time that it is called.*/
  T computeMaximumImageValue() const;
  
private:
  /** Linear interpolator used by getPixel functions. */
  typename LinearInterpolatorType::Pointer mLinearInterpolator;

  /** Nearest-neighbor interpolator used by getPixl functions. */
  typename NearestNeighborInterpolatorType::Pointer mNearestInterpolator;

  /** This is the itk::SmartPointer to the actual ITK image data. */
  typename itkImageType::Pointer mITKImage;

  /** A color that represents this image.  It could be used in GUI icons, for example. */
  QColor mColor;

  /** A name for this image. */
  QString mName;
  
  /** These members can act as VTK pipeline sources.  vtkImageImport is
      connected to itk::VTKImageExport. */
  vtkImageImport * mVTKImport;
  typename itk::VTKImageExport<itkImageType>::Pointer mITKExporter;
};

template<class T>
Image<T>::Image(itkImageType *img)
{
  mITKImage = img;
  
  // Hook up the ITK->VTK conversion pipeline
  this->constructVTKPipeline();
  
  // Create and hook up the interpolator objects.
  this->constructImageInterpolators();
}
  
template<class T>
bool Image<T>::read(QString fname)
{
  // Read ITK image data from a file
  typename itk::ImageFileReader<itkImageType>::Pointer reader =
    itk::ImageFileReader<itkImageType>::New();  
  reader->SetFileName(fname.toAscii());
  reader->Update();
  mITKImage = reader->GetOutput();

  // Hook up the ITK->VTK conversion pipeline
  this->constructVTKPipeline();

  // Create and hook up the interpolator objects.
  this->constructImageInterpolators();

  // Is image valid?
  if (!mITKImage)  {  return false;  }
  
  QFileInfo fi(fname);
  mName = fi.fileName();
  
  return true;
}

template<class T>
void Image<T>::constructVTKPipeline()
{  
  mVTKImport   = vtkImageImport::New();
  mITKExporter = itk::VTKImageExport<itkImageType>::New();
  mITKExporter->SetInput(mITKImage);
  ConnectPipelines(mITKExporter,mVTKImport);
  
  mVTKImport->Update();
}

template<class T>
void Image<T>::constructImageInterpolators()
{
  mLinearInterpolator = LinearInterpolatorType::New();
  mLinearInterpolator->SetInputImage(mITKImage);
  
  mNearestInterpolator = NearestNeighborInterpolatorType::New();
  mNearestInterpolator->SetInputImage(mITKImage);
}
  
template<class T>
bool Image<T>::write(QString fname) const
{
  typename itk::ImageFileWriter<itkImageType>::Pointer writer = 
    itk::ImageFileWriter<itkImageType>::New();

  writer->SetFileName(fname.toAscii());
  writer->SetInput(mITKImage);
  writer->Update();

  return true;
}

template<class T>
typename Image<T>::itkImageType::PixelType 
Image<T>::getPixel(int i, int j, int k)  const
{
  typename Image<T>::itkImageType::IndexType pixelIndex;
  pixelIndex[0] = i;
  pixelIndex[1] = j;
  pixelIndex[2] = k;
  typename Image<T>::itkImageType::PixelType pixelValue = mITKImage->GetPixel(pixelIndex);
  return pixelValue;
}

template<class T>
typename Image<T>::itkImageType::PixelType 
Image<T>::getPixel(int ijk[3]) const
{
  return this->getPixel(ijk[0], ijk[1], ijk[2]);
}

template<class T>
int Image<T>::getSliceForPoint(double point[3]) const
{

  typename itkImageType::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];

  typename itkImageType::IndexType index;
  mITKImage->TransformPhysicalPointToIndex(p, index);

  return index[2];
}

template<class T>
double Image<T>::getClosestSlicePoint(double point[3]) const
{
  typename itkImageType::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];

  typename itkImageType::IndexType index;
  mITKImage->TransformPhysicalPointToIndex(p, index);

  index[2]--;
  typename itkImageType::PointType newPoint;
  mITKImage->TransformIndexToPhysicalPoint(index, newPoint);

  return newPoint[2];
}

template<class T>
typename Image<T>::itkImageType::PixelType 
Image<T>::getLinearInterpolatedPixel(double point[3]) const
{
  typename itkImageType::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];
 
  typename LinearInterpolatorType::ContinuousIndexType index;

  mITKImage->TransformPhysicalPointToContinuousIndex(p, index);

  if (!mITKImage->GetLargestPossibleRegion().IsInside(index)) 
    {
    return 0.0f;
    }
  //mInterpolator->ConvertPointToContinuousIndex(p, index);
  return mLinearInterpolator->EvaluateAtContinuousIndex(index);
}

template<class T>
typename Image<T>::itkImageType::PixelType 
Image<T>::getNearestInterpolatedPixel(double point[3]) const
{
  typename itkImageType::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];
  
  typename NearestNeighborInterpolatorType::ContinuousIndexType index;

  mITKImage->TransformPhysicalPointToContinuousIndex(p, index);

  if (!mITKImage->GetLargestPossibleRegion().IsInside(index)) 
    {
      return 0.0f;
    }
  
  //mInterpolator->ConvertPointToContinuousIndex(p, index);
  return mNearestInterpolator->EvaluateAtContinuousIndex(index);
}

template<class T>
float Image<T>::getMinimumSpacing()  const
{
  const typename Image<T>::itkImageType::SpacingType& inputSpacing = mITKImage->GetSpacing();
  
  double isoSpacing = std::min(inputSpacing[0], inputSpacing[1]);
  isoSpacing = std::min(isoSpacing, inputSpacing[2]);
  return isoSpacing;
}

template<class T>
bool Image<T>::isBinarySegmentation()
{
  double *scalarRange = this->vtkImporter()->GetOutput()->GetScalarRange();

  if (scalarRange[0] != 0.0f || scalarRange[1] != 1.0f) 
    {  return false; } 
  else { return true; }
}
 
template<class T>
T Image<T>::computeMaximumImageValue() const
{
  itk::ImageRegionConstIterator<itkImageType> it(mITKImage, mITKImage->GetLargestPossibleRegion());
  
  T max = std::numeric_limits<T>::min();
  it.GoToBegin();
  while (! it.IsAtEnd() )
    {
      if (it.Value() > max) max = it.Value();
      ++it;
    }

  return max;
}


// Define some standard types.
typedef Image<float> FloatImage;
typedef Image<unsigned long int> ULongImage;
 
} // end namespace wse
#endif
