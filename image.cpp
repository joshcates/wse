#include "image.h"
#include <iostream>
#include "utils.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkAntiAliasBinaryImageFilter.h"
#include "itkReinitializeLevelSetImageFilter.h"

#include "itkRegionOfInterestImageFilter.h"
#include "vtkImageData.h"

Image::Image():
  mOriginal(NULL),
  mModified(NULL),
  mName(""),
  mVTKImportOriginal(NULL),
  mVTKImportModified(NULL),
  mImageGaussian(NULL)
{
  mColor = QColor(100,100,100);
  //  mColor = QColor(cvRandInt(&rng)%255,cvRandInt(&rng)%255,cvRandInt(&rng)%255);

}

Image::~Image()
{
  if (mVTKImportModified != NULL) mVTKImportModified->Delete();
  if (mVTKImportOriginal != NULL) mVTKImportOriginal->Delete();
}

bool Image::load(QString fname)
{
  itk::ImageFileReader<itkFloatImage>::Pointer reader =
    itk::ImageFileReader<itkFloatImage>::New();
  
  reader->SetFileName(fname.toAscii());
  reader->Update();

  mOriginal = reader->GetOutput();
  
  unsigned long x = mOriginal->GetLargestPossibleRegion().GetSize()[0];
  unsigned long y = mOriginal->GetLargestPossibleRegion().GetSize()[1];
  unsigned long z = mOriginal->GetLargestPossibleRegion().GetSize()[2];

  mOriginalRegion = mOriginal->GetLargestPossibleRegion();

    std::cerr << "\nLoaded image ('" << fname.toAscii().data() << "'\n"
    << "with dimensions: [" << x << ", " << y << ", " << z << "]\n";
  
  itkFloatImage::PointType origin = mOriginal->GetOrigin();

    std::cerr << "with origin (" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")\n";

  // Hook up the ITK->VTK conversion pipeline.
  mVTKImportOriginal   = vtkImageImport::New();
  mITKExporterOriginal = itk::VTKImageExport<itkFloatImage>::New();
  mITKExporterOriginal->SetInput(mOriginal);
  ConnectPipelines(mITKExporterOriginal,mVTKImportOriginal);

  mVTKImportOriginal->Update();
  mVTKImportResampled = NULL;

  vtkImageData *imageData = originalVTK()->GetOutput();
  double *scalarRange = imageData->GetScalarRange();
  std::cerr << "Scalar Range = [" << scalarRange[0] << ", " << scalarRange[1] << "]\n";

  mLinearInterpolator = LinearInterpolatorType::New();
  mLinearInterpolator->SetInputImage(mOriginal);

  mNearestInterpolator = NearestNeighborInterpolatorType::New();
  mNearestInterpolator->SetInputImage(mOriginal);

  if (!mOriginal) 
    {
      return false;
    }
  
  //  resetModified();
  
  QFileInfo fi(fname);
  mName = fi.fileName();
  
  return true;
}

Image::itkFloatImage::PixelType Image::getPixel(int i, int j, int k) 
{
  Image::itkFloatImage::IndexType pixelIndex;
  pixelIndex[0] = i;
  pixelIndex[1] = j;
  pixelIndex[2] = k;
  Image::itkFloatImage::PixelType pixelValue = mOriginal->GetPixel(pixelIndex);
  return pixelValue;
}

Image::itkFloatImage::PixelType Image::getPixel(int ijk[3]) 
{
  return getPixel(ijk[0], ijk[1], ijk[2]);
}

int Image::getSliceForPoint(double point[3]) 
{

  itkFloatImage::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];

  itkFloatImage::IndexType index;
  mOriginal->TransformPhysicalPointToIndex(p, index);

  return index[2];
}

double Image::getClosestSlicePoint(double point[3]) {

  itkFloatImage::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];

  itkFloatImage::IndexType index;
  mOriginal->TransformPhysicalPointToIndex(p, index);

  index[2]--;
  itkFloatImage::PointType newPoint;
  mOriginal->TransformIndexToPhysicalPoint(index, newPoint);

  return newPoint[2];
}


Image::itkFloatImage::PixelType  Image::getLinearInterpolatedPixel(double point[3])
{
  //return getGaussianPixel(point);

  itkFloatImage::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];

  
  LinearInterpolatorType::ContinuousIndexType index;

  mOriginal->TransformPhysicalPointToContinuousIndex(p, index);

  if (!mOriginalRegion.IsInside(index)) 
    {
    return 0.0f;
    }
  //mInterpolator->ConvertPointToContinuousIndex(p, index);
  return mLinearInterpolator->EvaluateAtContinuousIndex(index);
}

Image::itkFloatImage::PixelType Image::getNearestInterpolatedPixel(double point[3]) 
{
  //return getGaussianPixel(point);

  itkFloatImage::PointType p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];


  NearestNeighborInterpolatorType::ContinuousIndexType index;

  mOriginal->TransformPhysicalPointToContinuousIndex(p, index);

  if (!mOriginalRegion.IsInside(index)) {
    return 0.0f;
  }

  //mInterpolator->ConvertPointToContinuousIndex(p, index);
  return mNearestInterpolator->EvaluateAtContinuousIndex(index);
}



// Image::itkFloatImage::PixelType Image::getGaussianPixel( double point[3] )
// {
//   if (!mImageGaussian) 
//     {
//       generateGaussian();
//     }
  
//   itkFloatImage::PointType p;
//   p[0] = point[0];
//   p[1] = point[1];
//   p[2] = point[2];


//   LinearInterpolatorType::ContinuousIndexType index;

//   mOriginal->TransformPhysicalPointToContinuousIndex(p, index);

//   if (!mOriginalRegion.IsInside(index)) {
//     return 0.0f;
//   }

//   //mInterpolator->ConvertPointToContinuousIndex(p, index);
//   return mGaussianInterpolator->EvaluateAtContinuousIndex(index);
 
// }



// generate gaussian blurring of original image
// void Image::generateGaussian()
// {
//   mImageGaussian = itk::DiscreteGaussianImageFilter<itkFloatImage, itkFloatImage>::New();
//   mImageGaussian->SetInput(mOriginal);
//   mImageGaussian->SetVariance(0.5 * 0.5);
//   mImageGaussian->Update();

//   mGaussianInterpolator = LinearInterpolatorType::New();
//   mGaussianInterpolator->SetInputImage(mImageGaussian->GetOutput());

// }




//vtkImageImport *Image::resampledVTK(SmoothSettings settings) 
//{ 
//   if (settings != lastSmoothSettings || mVTKImportResampled == NULL) {
//     generateResampledImage(settings);
//     lastSmoothSettings = settings;
//   }
//   return mVTKImportResampled; 
// }


// void Image::generateResampledImage(SmoothSettings settings) 
// {
//   try 
//     {
      
//       std::cerr << "\nSmoothing image...\n";
//       const     unsigned int   Dimension = 3;
//       //typedef   float InputPixelType;
//       //typedef   float OutputPixelType;
      
//       typedef Image::itkFloatImage ImageType;
//       typedef Image::itkFloatImage InputImageType;
//       typedef Image::itkFloatImage OutputImageType;
//       typedef itk::ImageFileWriter< OutputImageType >  WriterType;
//       WriterType::Pointer writer;
//       typedef ImageType::RegionType RegionType;
      
//       itk::ImageSource<OutputImageType>::OutputImageType *output;
//       //OutputImageType *output;
      
      
//       InputImageType::Pointer inputImage = mOriginal;
//       InputImageType::RegionType originalRegion = inputImage->GetLargestPossibleRegion();
      
//       // bounding box
//       itk::ImageRegionIteratorWithIndex<ImageType> it(mOriginal, mOriginal->GetLargestPossibleRegion());
      
      
//       RegionType::SizeType  imgSize   = originalRegion.GetSize();
//       RegionType::IndexType imgCorner = originalRegion.GetIndex();
      
//       // Search for bounding box.
//       ImageType::IndexType idx;
//       ImageType::IndexType lower = imgCorner + imgSize;
//       ImageType::IndexType upper = imgCorner;
      
//       for (it.GoToBegin(); ! it.IsAtEnd(); ++it) {
// 	if (it.Get() > 0.0f) {
// 	  idx = it.GetIndex();
	  
// 	  for (unsigned int i = 0; i < Dimension; i++) {
// 	    if (lower[i] > idx[i]) {
// 	      lower[i] = idx[i] - 2;
// 	    }
// 	    if (upper[i] < idx[i]) {
// 	      upper[i] = idx[i] + 2;
// 	    }
// 	  }
// 	}
//       }
      
//       RegionType::SizeType regionSize;
//       for (unsigned int i = 0; i <Dimension; i++) {
// 	regionSize[i] = upper[i] - lower[i];
//       }
      
//       InputImageType::RegionType regionOfInterest;
//       regionOfInterest.SetSize(regionSize);
//       regionOfInterest.SetIndex(lower);
      
      
      
//       typedef itk::RegionOfInterestImageFilter< InputImageType,
// 						OutputImageType > RegionFilterType;
      
//       RegionFilterType::Pointer regionFilter = RegionFilterType::New();
//       regionFilter->SetRegionOfInterest(regionOfInterest);
//       regionFilter->SetInput(mOriginal);
//       regionFilter->Update();
      
//       //writer = WriterType::New();
//       //writer->SetFileName("/tmp/step0-crop.nrrd");
//       //writer->SetInput(regionFilter->GetOutput());
//       //writer->Update();
//       //std::cerr << "Step 0: cropping written\n";
//       //regionFilter->ReleaseDataFlagOn(); 
      
      
//       // resample filter
//       typedef itk::ResampleImageFilter<InputImageType, OutputImageType > FilterType;
//       FilterType::Pointer resampler = FilterType::New();
      
      
//       typedef itk::IdentityTransform< double, Dimension >  TransformType;
//       TransformType::Pointer transform = TransformType::New();
      
//       // use nearest neighbor, not linear
//       typedef itk::NearestNeighborInterpolateImageFunction<InputImageType, double >  nnInterpolatorType;
//       //typedef itk::LinearInterpolateImageFunction<InputImageType, double >  nnInterpolatorType;
//       nnInterpolatorType::Pointer nnInterpolator = nnInterpolatorType::New();
      
      
//       resampler->SetInterpolator(nnInterpolator);
//       //filter->SetDefaultPixelValue( 100 );
//       resampler->SetDefaultPixelValue( 0 );
      
      
//       //resampler->SetOutputOrigin(inputImage->GetOrigin());
//       resampler->SetOutputOrigin(regionFilter->GetOutput()->GetOrigin());
      
      
//       //const InputImageType::SpacingType& inputSpacing = inputImage->GetSpacing();
//       const InputImageType::SpacingType& inputSpacing = regionFilter->GetOutput()->GetSpacing();
      
//       double isoSpacing = inputSpacing[0];
//       std::cerr << "original spacing: [" << inputSpacing[0] << ", " << 
// 	inputSpacing[1] << ", " << inputSpacing[2] << "]\n";
      
//       std::cerr << "new spacing: [" << isoSpacing << ", " << isoSpacing << ", " << isoSpacing << "]\n";
      
      
//       OutputImageType::SpacingType spacing;
      
//       spacing[0] = isoSpacing;
//       spacing[1] = isoSpacing;
//       spacing[2] = isoSpacing;
      
//       resampler->SetOutputSpacing(spacing);
      
//       InputImageType::RegionType region = regionFilter->GetOutput()->GetLargestPossibleRegion();
      
      
//       InputImageType::SizeType   inputSize = region.GetSize();
//       InputImageType::IndexType  start     = region.GetIndex();
      
//       typedef InputImageType::SizeType::SizeValueType SizeValueType;
      
      
//       // compute the new size based on the difference in spacing
//       double dx = inputSize[0];
//       double dy = inputSize[1];
//       double dz = inputSize[2] * inputSpacing[2] / isoSpacing;
      
//       std::cerr << "original (cropped) size: [" << inputSize[0] << ", " << 
// 	inputSize[1] << ", " << inputSize[2] << "]\n";
      
//       std::cerr << "new size: [" << dx << ", " << dy << ", " << dz << "]\n";
      
      
//       InputImageType::SizeType size;
      
//       size[0] = static_cast<SizeValueType>( dx );
//       size[1] = static_cast<SizeValueType>( dy );
//       size[2] = static_cast<SizeValueType>( dz );
      
      
//       resampler->SetSize( size );
//       resampler->SetOutputStartIndex( start );
      
//       //resampler->SetInput( mOriginal );
//       resampler->SetInput( regionFilter->GetOutput() );
      
//       transform->SetIdentity();
//       resampler->SetTransform( transform );
      
//       //filter->Update();

      
      
//       //writer = WriterType::New();
//       //writer->SetFileName("/tmp/step1-resample.nrrd");
//       //writer->SetInput(resampler->GetOutput());
//       //writer->Update();
//       //std::cerr << "Step 1: resampling written\n";
//       resampler->ReleaseDataFlagOn(); 
      
      
//       output = resampler->GetOutput();
      
//       itk::AntiAliasBinaryImageFilter<Image::itkFloatImage, Image::itkFloatImage>::Pointer anti
// 	= itk::AntiAliasBinaryImageFilter<Image::itkFloatImage, Image::itkFloatImage>::New();
      
      
//       int numIter = settings.mAntiAliasIterations;
      
      
//       if (settings.mAntiAliasing) {
// 	//anti->SetInput(mOriginal);
// 	anti->SetInput(output);
// 	anti->SetNumberOfIterations(settings.mAntiAliasIterations);
// 	anti->SetMaximumRMSError(0.0);
// 	output = anti->GetOutput();
	
	
	
// 	//anti->Update();
	
// 	//writer = WriterType::New();
// 	//writer->SetFileName("/tmp/step2-antialias.nrrd");
// 	//writer->SetInput(anti->GetOutput());
// 	//writer->Update();
// 	//std::cerr << "Step 2: anti-aliasing written\n";
// 	anti->ReleaseDataFlagOn();
//       }
      
      
//       itk::ReinitializeLevelSetImageFilter<itkFloatImage>::Pointer filt
// 	= itk::ReinitializeLevelSetImageFilter<itkFloatImage>::New();
//       filt->SetInput(output);
//       filt->NarrowBandingOff();
//       filt->SetLevelSetValue(0.0);
      
//       //writer = WriterType::New();
//       //writer->SetFileName("/tmp/step3-levelset.nrrd");
//       //writer->SetInput(filt->GetOutput());
//       //writer->Update();
//       //std::cerr << "Step 3: level set done\n";
//       filt->ReleaseDataFlagOn(); 
//       output = filt->GetOutput();
      
      
//       itk::DiscreteGaussianImageFilter<itkFloatImage, itkFloatImage>::Pointer blur
// 	= itk::DiscreteGaussianImageFilter<itkFloatImage, itkFloatImage>::New();
      
//       if (settings.mGaussianBlurring) {
// 	blur->SetInput(output);
// 	blur->SetVariance(settings.mGaussianVariance * settings.mGaussianVariance);
// 	//blur->SetVariance(1.5 * 1.5);
// 	//blur->SetVariance(13.5 * 13.5);
// 	blur->Update();
// 	output = blur->GetOutput();
	
// 	//writer = WriterType::New();
// 	//writer->SetFileName("/tmp/step4-gaussian.nrrd");
// 	//writer->SetInput(filt->GetOutput());
// 	//writer->Update();
// 	//std::cerr << "Step 4: gaussian done\n";
// 	blur->ReleaseDataFlagOn();
//       }
           
//       output->Update();
      
//       // Hook up the ITK->VTK conversion pipeline.
//       mVTKImportResampled = vtkImageImport::New();
//       mITKExporterResampled = itk::VTKImageExport<itkFloatImage>::New();
//       mITKExporterResampled->SetInput(output);
//       ConnectPipelines(mITKExporterResampled,mVTKImportResampled);
      
//       std::cerr << "Done smoothing image\n\n";
      
//     }
//   catch( itk::ExceptionObject & excep )
//     {
//       std::cerr << "Exception caught!" << std::endl;
//       std::cerr << excep << std::endl;
//     }
// }

float Image::getMinimumSpacing () {
  
  const Image::itkFloatImage::SpacingType& inputSpacing = mOriginal->GetSpacing();
  
  double isoSpacing = std::min(inputSpacing[0], inputSpacing[1]);
  isoSpacing = std::min(isoSpacing, inputSpacing[2]);
  return isoSpacing;
}

bool Image::isBinarySegmentation()
{
  double *scalarRange = this->originalVTK()->GetOutput()->GetScalarRange();

  if (scalarRange[0] != 0.0f || scalarRange[1] != 1.0f) {
    return false;
  } else {
    return true;
  }
}
