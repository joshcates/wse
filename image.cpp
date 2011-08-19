#include "image.h"

namespace wse {

Image::Image(itkFloatImage *img)
{
  mITKImage = img;
  
  // Hook up the ITK->VTK conversion pipeline
  this->constructVTKPipeline();
  
  // Create and hook up the interpolator objects.
  this->constructImageInterpolators();
}
  
bool Image::read(QString fname)
{
  // Read ITK image data from a file
  itk::ImageFileReader<itkFloatImage>::Pointer reader =
    itk::ImageFileReader<itkFloatImage>::New();  
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

void Image::constructVTKPipeline()
{  
  mVTKImport   = vtkImageImport::New();
  mITKExporter = itk::VTKImageExport<itkFloatImage>::New();
  mITKExporter->SetInput(mITKImage);
  ConnectPipelines(mITKExporter,mVTKImport);
  
  mVTKImport->Update();
}

void Image::constructImageInterpolators()
{
  mLinearInterpolator = LinearInterpolatorType::New();
  mLinearInterpolator->SetInputImage(mITKImage);
  
  mNearestInterpolator = NearestNeighborInterpolatorType::New();
  mNearestInterpolator->SetInputImage(mITKImage);
}
  
  bool Image::write(QString fname) const
{
  itk::ImageFileWriter<itkFloatImage>::Pointer writer = 
    itk::ImageFileWriter<itkFloatImage>::New();

  writer->SetFileName(fname.toAscii());
  writer->SetInput(mITKImage);
  writer->Update();

  return true;
}

Image::itkFloatImage::PixelType Image::getPixel(int i, int j, int k)  const
{
  Image::itkFloatImage::IndexType pixelIndex;
  pixelIndex[0] = i;
  pixelIndex[1] = j;
  pixelIndex[2] = k;
  Image::itkFloatImage::PixelType pixelValue = mITKImage->GetPixel(pixelIndex);
  return pixelValue;
}

Image::itkFloatImage::PixelType Image::getPixel(int ijk[3]) const
{
  return this->getPixel(ijk[0], ijk[1], ijk[2]);
}

int Image::getSliceForPoint(double point[3]) const
{

  itkFloatImage::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];

  itkFloatImage::IndexType index;
  mITKImage->TransformPhysicalPointToIndex(p, index);

  return index[2];
}

double Image::getClosestSlicePoint(double point[3]) const
{
  itkFloatImage::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];

  itkFloatImage::IndexType index;
  mITKImage->TransformPhysicalPointToIndex(p, index);

  index[2]--;
  itkFloatImage::PointType newPoint;
  mITKImage->TransformIndexToPhysicalPoint(index, newPoint);

  return newPoint[2];
}

Image::itkFloatImage::PixelType Image::getLinearInterpolatedPixel(double point[3]) const
{
  itkFloatImage::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];
 
  LinearInterpolatorType::ContinuousIndexType index;

  mITKImage->TransformPhysicalPointToContinuousIndex(p, index);

  if (!mITKImage->GetLargestPossibleRegion().IsInside(index)) 
    {
    return 0.0f;
    }
  //mInterpolator->ConvertPointToContinuousIndex(p, index);
  return mLinearInterpolator->EvaluateAtContinuousIndex(index);
}

Image::itkFloatImage::PixelType Image::getNearestInterpolatedPixel(double point[3]) const
{
  itkFloatImage::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];
  
  NearestNeighborInterpolatorType::ContinuousIndexType index;

  mITKImage->TransformPhysicalPointToContinuousIndex(p, index);

  if (!mITKImage->GetLargestPossibleRegion().IsInside(index)) 
    {
      return 0.0f;
    }
  
  //mInterpolator->ConvertPointToContinuousIndex(p, index);
  return mNearestInterpolator->EvaluateAtContinuousIndex(index);
}

float Image::getMinimumSpacing()  const
{
  const Image::itkFloatImage::SpacingType& inputSpacing = mITKImage->GetSpacing();
  
  double isoSpacing = std::min(inputSpacing[0], inputSpacing[1]);
  isoSpacing = std::min(isoSpacing, inputSpacing[2]);
  return isoSpacing;
}

bool Image::isBinarySegmentation()
{
  double *scalarRange = this->vtkImporter()->GetOutput()->GetScalarRange();

  if (scalarRange[0] != 0.0f || scalarRange[1] != 1.0f) 
    {  return false; } 
  else { return true; }
}
 

} // end namespace wse
