

#include "IsoRenderer.h"

#include "vtkImageActor.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"
#include "vtkHedgeHog.h"
#include "vtkImageData.h"
#include "vtkLinearSubdivisionFilter.h"
#include "vtkImageConstantPad.h"

#include <QTime>

const std::string IsoRenderer::RENDER_HIDDEN("Not Shown");
const std::string IsoRenderer::RENDER_SURFACE("Show as Surface");
const std::string IsoRenderer::RENDER_MESH("Show as Mesh");
const std::string IsoRenderer::RENDER_POINTS("Show as Points");

IsoRenderer::IsoRenderer() :
mWallRenderType(RENDER_SURFACE), mEndoRenderType(RENDER_SURFACE),
mClipDirection(true), mShowNormals(false), mClipSurfaceToSlice(false),
mSliceVisible(true), mBackgroundColorRed(0.0), mBackgroundColorGreen(0.0),
mBackgroundColorBlue(0.0), mScalarOnWall(true), mCameraSet(false),
mDataLinearInterpolation(false), mMaskLinearInterpolation(false),
mScalarMethod(ScalarMethod::NORMAL_MAXIMUM_INTENSITY),
mImageData(NULL), mEndoData(NULL), mWallData(NULL),
mScalarMin(0), mScalarMax(0), mSubDivideMesh(false), mSmoothThreshold(false)
{

  mShowNormals = false;
  mClipSurfaceToSlice = false;
  mScalarOnWall = true;

  installPipeline();
}


void IsoRenderer::installPipeline()
{

  mEndoContourFilter = vtkContourFilter::New();
  mEndoContourFilter->SetValue(0, 0.5);
  mEndoContourFilter->SetNumberOfContours(1);

  mEndoMarchingCubes = vtkMarchingCubes::New();
  mEndoMarchingCubes->SetValue(0,0.5);
  mEndoMarchingCubes->SetNumberOfContours(1);

  mWallContourFilter = vtkContourFilter::New();
  mWallContourFilter->SetValue(0, 0.5);
  mWallContourFilter->SetNumberOfContours(1);

  mWallMarchingCubes = vtkMarchingCubes::New();
  mWallMarchingCubes->SetValue(0,0.5);
  mWallMarchingCubes->SetNumberOfContours(1);


  mEndoPolyDataNormals = vtkPolyDataNormals::New();
  //mPolyDataNormals->SetInput(mContourFilter->GetOutput());
  //mPolyDataNormals->SetFeatureAngle(45);
  //mPolyDataNormals->SplittingOn();
  mEndoPolyDataNormals->SplittingOff();


  mWallPolyDataNormals = vtkPolyDataNormals::New();
  mWallPolyDataNormals->SplittingOff();


  //vtkSmoothPolyDataFilter *smoothPolyDataFilter = vtkSmoothPolyDataFilter::New();
  //smoothPolyDataFilter->SetInputConnection(mPolyDataNormals->GetOutputPort());
  //smoothPolyDataFilter->GenerateErrorScalarsOn();
  //smoothPolyDataFilter->SetNumberOfIterations(50);
  //smoothPolyDataFilter->SetRelaxationFactor(0.01f);
  //smoothPolyDataFilter->SetFeatureAngle(60);



  mScalarLUT = vtkLookupTable::New();
  mScalarLUT->SetNumberOfColors(1001);
  // sort of a blue green range, but it has teal in-between
  mScalarLUT->SetHueRange(0.66,0.33);
  mScalarLUT->Build();
  mScalarLUT->SetTableValue(0,0.4f,0.4f,0.4f,1.0f);


  mEndoContourMapper = vtkPolyDataMapper::New();
  mEndoContourMapper->SetInput(mEndoPolyDataNormals->GetOutput());
  mEndoContourMapper->ScalarVisibilityOn();
  mEndoContourMapper->SetLookupTable(mScalarLUT);
  mEndoContourMapper->SetScalarRange(0,1001);

  mWallContourMapper = vtkPolyDataMapper::New();
  mWallContourMapper->SetInput(mWallPolyDataNormals->GetOutput());
  mWallContourMapper->ScalarVisibilityOn();
  mWallContourMapper->SetLookupTable(mScalarLUT);
  mWallContourMapper->SetScalarRange(0,1001);


  mHedgeHogMapper = vtkPolyDataMapper::New();


  // Rendering objects.


  mHedgeHogActor = vtkActor::New();

  mEndoSurfaceActor = vtkActor::New();    
  mWallSurfaceActor = vtkActor::New();    
  // mSurfaceActor->GetProperty()->SetSpecularColor(0.0, 0.0, 1.0);
  // mSurfaceActor->GetProperty()->SetDiffuse(0.0);
  // mSurfaceActor->GetProperty()->SetSpecular(0.8);
  // mSurfaceActor->GetProperty()->SetSpecularPower(5.0);


  mEndoSurfaceActor->SetMapper(mEndoContourMapper);    
  mWallSurfaceActor->SetMapper(mWallContourMapper);    

  mSliceActor = vtkImageActor::New();

  // disable interpolation so that we can see each pixel easily
  mSliceActor->SetInterpolate(0);


  // create stencil LUT for the mask
  mSliceStencilLUT = vtkLookupTable::New();
  mSliceStencilLUT->SetRange(0,1);
  mSliceStencilLUT->SetNumberOfColors(2);
  mSliceStencilLUT->Build();
  mSliceStencilLUT->SetTableValue(0,0,0,0,0);
  mSliceStencilLUT->SetTableValue(1,0,0,0,1);

  mSliceStencilMap = vtkImageMapToColors::New();
  mSliceStencilBlend = vtkImageBlend::New();


  mClipPlane = vtkPlane::New();

  mSurfaceRenderer = vtkRenderer::New();


}

vtkRenderer * IsoRenderer::getSurfaceRenderer()
{
  return mSurfaceRenderer;
}

IsoRenderer::~IsoRenderer()
{
  // uninstall the pipeline
}

void IsoRenderer::setImageData( Image *image )
{
  mImageData = image;
}

void IsoRenderer::setWallData( Image *image )
{
  mWallData = image;
}

void IsoRenderer::setEndoData( Image *image )
{
  mEndoData = image;
}

void IsoRenderer::createIsoSurfaces()
{
  mSurfaceRenderer->RemoveActor(mWallSurfaceActor);
  mSurfaceRenderer->RemoveActor(mEndoSurfaceActor);
  mSurfaceRenderer->RemoveActor(mSliceActor);

  if (mImageData == NULL || mWallData == NULL || mEndoData == NULL) {
    return;
  }


  vtkImageImport *endoImage;
  vtkImageImport *wallImage;
  if (mSmoothSettings.mEnabled) {

    //VTK way of smoothing
    //vtkImageGaussianSmooth *gaussian = vtkImageGaussianSmooth::New();
    //gaussian->SetDimensionality(3);
    //gaussian->SetStandardDeviation(2,2,2);
    //gaussian->SetRadiusFactor(1);
    //gaussian->SetInputConnection(mImageStack->image(mIsosurfaceImage)->originalVTK()->GetOutputPort());
    //mContourFilter->SetInputConnection(gaussian->GetOutputPort());

    endoImage = mEndoData->resampledVTK(mSmoothSettings);
    wallImage = mWallData->resampledVTK(mSmoothSettings);
  } else {
    endoImage = mEndoData->originalVTK();
    wallImage = mWallData->originalVTK();
  }


  vtkPolyData *endoPolyData;
  vtkPolyData *wallPolyData;
  int volextent[6];

  // constant pad for endo
  vtkImageConstantPad *endoConstantPad = vtkImageConstantPad::New();
  endoConstantPad->SetInputConnection(endoImage->GetOutputPort());
  endoConstantPad->SetConstant(0); 

  endoImage->GetWholeExtent(volextent);
  volextent[0]-=1; volextent[1]+=1; volextent[2]-=1;
  volextent[3]+=1; volextent[4]-=1; volextent[5]+=1;
  endoConstantPad->SetOutputWholeExtent(volextent);
  endoConstantPad->Update();

  // constant pad for wall
  vtkImageConstantPad *wallConstantPad = vtkImageConstantPad::New();
  wallConstantPad->SetInputConnection(wallImage->GetOutputPort());
  wallConstantPad->SetConstant(0); 

  wallImage->GetWholeExtent(volextent);
  volextent[0]-=1; volextent[1]+=1; volextent[2]-=1;
  volextent[3]+=1; volextent[4]-=1; volextent[5]+=1;
  wallConstantPad->SetOutputWholeExtent(volextent);
  wallConstantPad->Update();


  // removed this option from the UI since they both look the same to me.
  //if (ui.visMethodCB->currentIndex() == 0) {
  if (0) {
    mEndoContourFilter->SetInputConnection(endoConstantPad->GetOutputPort());
    mWallContourFilter->SetInputConnection(wallConstantPad->GetOutputPort());
    endoPolyData = mEndoContourFilter->GetOutput();
    wallPolyData = mWallContourFilter->GetOutput();
  } else {
    //mEndoMarchingCubes->SetInputConnection(endoImage);
    mEndoMarchingCubes->SetInputConnection(endoConstantPad->GetOutputPort());
    mWallMarchingCubes->SetInputConnection(wallConstantPad->GetOutputPort());
    endoPolyData = mEndoMarchingCubes->GetOutput();
    wallPolyData = mWallMarchingCubes->GetOutput();
  }


  std::cerr << "\nComputing isosurfaces...\n";

  QTime startTime =  QTime::currentTime();

  // Mesh subdivision
  if (mSubDivideMesh) {
    vtkLinearSubdivisionFilter *subdivisionFilter = vtkLinearSubdivisionFilter::New();
    //vtkButterflySubdivisionFilter *subdivisionFilter = vtkButterflySubdivisionFilter::New();
    //vtkLoopSubdivisionFilter *subdivisionFilter = vtkLoopSubdivisionFilter::New();
    //subdivisionFilter->SetNumberOfSubdivisions(1);
    subdivisionFilter->SetInput(wallPolyData);
    wallPolyData = subdivisionFilter->GetOutput();
  }

  mWallPolyDataNormals->SetInput(wallPolyData);
  mEndoPolyDataNormals->SetInput(endoPolyData);

  mEndoPolyDataNormals->Update();
  mWallPolyDataNormals->Update();

  qint32 msecs = startTime.msecsTo( QTime::currentTime() );
  std::cerr << "Isosurface generation took " << msecs << "ms\n";

  updateRenderType();


  updateSlice();
  updateClipPlane();

  updateClipToSurface();
  updateSliceVisibility();


  // update colors
  updateBackgroundColor();

  /*********************************************************/


  double minx, miny, minz, maxx, maxy, maxz;
  minx = miny = minz = std::numeric_limits<float>::max();
  maxx = maxy = maxz = std::numeric_limits<float>::min();

  vtkPoints *points = endoPolyData->GetPoints();
  std::cerr << "isosurface: number of points (endo): " << points->GetNumberOfPoints() << "\n";

  points = wallPolyData->GetPoints();
  std::cerr << "isosurface: number of points (wall): " << points->GetNumberOfPoints() << "\n";

  updateIsoScalars();

  //mSurfaceRenderer->ResetCamera();
  if (!mCameraSet) { // only do this once, but we should probably redo it if the isosurface source changes
    mSurfaceRenderer->ResetCamera(mEndoContourMapper->GetBounds());
    mCameraSet = true;
  }

  mSurfaceRenderer->Render();

}




void IsoRenderer::updateRenderType() 
{
  mSurfaceRenderer->RemoveActor(mWallSurfaceActor);
  mSurfaceRenderer->RemoveActor(mEndoSurfaceActor);

  // set the render type for the endo
  if (mEndoRenderType == RENDER_SURFACE) {
    mEndoSurfaceActor->GetProperty()->SetRepresentationToSurface();
  } else if (mEndoRenderType == RENDER_MESH) {
    mEndoSurfaceActor->GetProperty()->SetRepresentationToWireframe();
  } else if (mEndoRenderType == RENDER_POINTS) {
    mEndoSurfaceActor->GetProperty()->SetRepresentationToPoints();
  }

  if (mEndoRenderType != RENDER_HIDDEN) {
    mSurfaceRenderer->AddActor(mEndoSurfaceActor);
  }

  // set the render type for the wall
  if (mWallRenderType == RENDER_SURFACE) {
    mWallSurfaceActor->GetProperty()->SetRepresentationToSurface();
  } else if (mWallRenderType == RENDER_MESH) {
    mWallSurfaceActor->GetProperty()->SetRepresentationToWireframe();
  } else if (mWallRenderType == RENDER_POINTS) {
    mWallSurfaceActor->GetProperty()->SetRepresentationToPoints();
  }

  if (mWallRenderType != RENDER_HIDDEN) {
    mSurfaceRenderer->AddActor(mWallSurfaceActor);
  }   
}

void IsoRenderer::setRenderMethods( std::string wallRenderType, std::string endoRenderType )
{
  mWallRenderType = wallRenderType;
  mEndoRenderType = endoRenderType;
  updateRenderType();
}


void IsoRenderer::updateSlice()
{
  // vtkImageMapToColors for the mask
  mSliceStencilMap->RemoveAllInputs();
  mSliceStencilMap->SetInputConnection(mWallData->originalVTK()->GetOutputPort());
  mSliceStencilMap->SetLookupTable(mSliceStencilLUT);
  mSliceStencilMap->PassAlphaToOutputOn();
  mSliceStencilMap->SetOutputFormatToRGBA();

  // a vtkImageBlend to clip the threshold image against the mask
  mSliceStencilBlend->RemoveAllInputs();
  mSliceStencilBlend->AddInputConnection(mSliceStencilMap->GetOutputPort());
  mSliceStencilBlend->AddInputConnection(mSlice);

  mSliceActor->SetInput(mSliceStencilBlend->GetOutput());
}

void IsoRenderer::setSlice( vtkAlgorithmOutput *slice )
{
  mSlice = slice;
}

void IsoRenderer::updateClipPlane() {
  double bounds[6];
  mSliceActor->GetBounds(bounds);
  if (mClipDirection) {
    mClipPlane->SetNormal(0,0,1);
  } else {
    mClipPlane->SetNormal(0,0,-1);
  }
  mClipPlane->SetOrigin(0,0,bounds[4]);
}

void IsoRenderer::setClipDirection( bool direction )
{
  mClipDirection = direction;
  updateClipPlane();
}

void IsoRenderer::setShowNormals( bool showNormals )
{
  mShowNormals = showNormals;
}


void IsoRenderer::setClipSurfaceToSlice( bool clipSurface )
{
  if (mClipSurfaceToSlice != clipSurface) {
    mClipSurfaceToSlice = clipSurface;
    updateClipToSurface();
  }
}

void IsoRenderer::updateClipToSurface()
{
  mEndoContourMapper->RemoveAllClippingPlanes();
  mWallContourMapper->RemoveAllClippingPlanes();
  mHedgeHogMapper->RemoveAllClippingPlanes();
  if (mClipSurfaceToSlice) {
    mEndoContourMapper->AddClippingPlane(mClipPlane);
    mWallContourMapper->AddClippingPlane(mClipPlane);
    mHedgeHogMapper->AddClippingPlane(mClipPlane);
  } 
}


void IsoRenderer::updateSliceVisibility() {
  mSurfaceRenderer->RemoveActor(mSliceActor);
  if (mSliceVisible) {
    mSurfaceRenderer->AddActor(mSliceActor);
  }
}

void IsoRenderer::setSliceVisibility( bool sliceVisible )
{
  mSliceVisible = sliceVisible;
  updateSliceVisibility();
}

void IsoRenderer::setBackgroundColor( float r, float g, float b )
{
  mBackgroundColorRed = r;
  mBackgroundColorGreen = g;
  mBackgroundColorBlue = b;
  updateBackgroundColor();
}

void IsoRenderer::updateBackgroundColor()
{
  mSurfaceRenderer->SetBackground(mBackgroundColorRed, mBackgroundColorGreen, mBackgroundColorBlue);
  mSurfaceRenderer->Render();
}


void IsoRenderer::updateIsoScalars() {
  if (mScalarOnWall) {
    setIsoScalars(mWallPolyDataNormals, true);
    eraseIsoScalars(mEndoPolyDataNormals);
  } else {
    setIsoScalars(mEndoPolyDataNormals, false);
    eraseIsoScalars(mWallPolyDataNormals);
  }
  //CHECK: updateColorMap();
}

void IsoRenderer::setIsoScalars(vtkPolyDataNormals *polyDataNormals, bool negativeNormal) 
{

  if (mImageData == NULL || mWallData == NULL || mEndoData == NULL) {
    // requires all three
    return;
  }

  std::cerr << "\nComputing scalars...\n";

  vtkImageData *imageData = mImageData->originalVTK()->GetOutput();

  vtkPolyData *polyData = polyDataNormals->GetOutput();
  if (!polyData || !polyData->GetPointData() || !polyData->GetPointData()->GetNormals()) {
    std::cerr << "\nError: Bad poly data (too much smoothing?)\n";
    return;
  }

  vtkFloatArray *pointNormalArray = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetNormals());
  std::cerr << "normals: number of tuples: " << pointNormalArray->GetNumberOfTuples() << "\n";

  vtkPoints *points = polyData->GetPoints();
  vtkFloatArray *normals = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetNormals());
  if (normals == NULL) {
    return;
  }

  int numPoints = normals->GetNumberOfTuples();


  vtkFloatArray *scalars = vtkFloatArray::New();
  scalars->SetNumberOfValues(numPoints);


  float maxValueEncountered = std::numeric_limits<float>::min();
  float minValueEncountered = std::numeric_limits<float>::max();


  double minimumSpacing = mImageData->getMinimumSpacing();

  for (int i=0; i < numPoints; i++) {
    double *normal = normals->GetTuple(i);
    double *point = points->GetPoint(i);

    double x = point[0];
    double y = point[1];
    double z = point[2];

    // adjust normal for spacing
    normal[0] = normal[0] * minimumSpacing;
    normal[1] = normal[1] * minimumSpacing;
    normal[2] = normal[2] * minimumSpacing;

    int nVolIdx = imageData->FindPoint(x,y,z);
    double *p = imageData->GetPoint(nVolIdx);

    float value = 0.0f;

    if (std::abs(normal[0]) + std::abs(normal[1]) + std::abs(normal[2]) < 0.5f) {
      std::cerr << "Uh oh, bad normal found for point " 
        << i << ", x: " << normal[0] << ", y: " << normal[1] << ", z: " << normal[2] << "\n";
    } else {
      value = computeIsoScalar(point, normal, negativeNormal);
    }

    scalars->SetValue(i, value);

    if (value >= 0.0f) {
      minValueEncountered = std::min(minValueEncountered, value);
      maxValueEncountered = std::max(maxValueEncountered, value);
    }
  }

  // clamp to 0, negative means a miss on the mask
  minValueEncountered = std::max(minValueEncountered, 0.0f);

  if (minValueEncountered < 0.00001f) { // below some epsilon drop to zero
    minValueEncountered = 0.0f;
  }

  std::cerr << "min scalar encountered: " << minValueEncountered << "\n";
  std::cerr << "max scalar encountered: " << maxValueEncountered << "\n";

  mScalarMax = maxValueEncountered;
  mScalarMin = minValueEncountered;

  // shift by one
  maxValueEncountered++;

  float range = maxValueEncountered - minValueEncountered;

  // rescale scalars based on the max value
  for (int i=0; i < numPoints; i++) {
    float value = scalars->GetValue(i);

    value = (value - minValueEncountered) / range * 1000.0f;

    // shift by one
    value++; 

    if (value < 0.0f) {
      value = 0.0f;
    }
    scalars->SetValue(i, value);
    //std::cerr << "Value = " << value << "\n";
  }



  polyData->GetPointData()->SetScalars(scalars);
  polyData->Update();

  updateHedgeHogs();
}

void IsoRenderer::eraseIsoScalars(vtkPolyDataNormals *polyDataNormals) {
  if (mImageData == NULL || mWallData == NULL || mEndoData == NULL) {
    // requires all three
    return;
  }

  vtkPolyData *polyData = polyDataNormals->GetOutput();
  vtkPoints *points = polyData->GetPoints();

  vtkFloatArray *normals = vtkFloatArray::SafeDownCast(polyData->GetPointData()->GetNormals());
  if (normals == NULL) {
    return;
  }

  int numPoints = normals->GetNumberOfTuples();

  vtkFloatArray *scalars = vtkFloatArray::New();
  scalars->SetNumberOfValues(numPoints);

  // rescale scalars based on the max value
  for (int i=0; i < numPoints; i++) {
    scalars->SetValue(i, 0);
  }

  polyData->GetPointData()->SetScalars(scalars);
  polyData->Update();

}


float IsoRenderer::computeIsoScalar(double *point, double *normal, bool negativeNormal) {
  float imageValue = 0.0f;
  //float isoValue = 1.0f;
  float maskValue = 1.0f; // start "in" the mask

  vtkImageData *imageData = mImageData->originalVTK()->GetOutput();

  //int ijk[3];
  //double pcoords[] = { 0.0f, 0.0f, 0.0f };
  //imageData->ComputeStructuredCoordinates(point, ijk, pcoords);

  double p[3];

  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];  

  int stepsTraveled = 0;
  float maxIntensity = 0.0f;
  float numIncluded = 0.0f;
  float numExcluded = 0.0f;

  

  //maskValue = maskImage->getInterpolatedPixel(p);
  //imageValue = image->getInterpolatedPixel(p);
  maskValue = getInterpolatedValue(mWallData, p, mMaskLinearInterpolation);
  imageValue = getInterpolatedValue(mImageData, p, mDataLinearInterpolation);
  float sumIncluded = 0.0f;
  float sumExcluded = 0.0f;


  float returnValue = -1.0f;

  int maskCount = 0;

  float maskCheck = 0.25f;

  const int maxSteps = 10;

  if (mScalarMethod.alongNormal == true) {
    while ((maskValue >= maskCheck || stepsTraveled < 3) && stepsTraveled < maxSteps) { // while still inside the mask

      if (maskValue >= maskCheck) {
        // maximum intensity value, but only if above threshold
        float value = applyThreshold(imageValue);
        if (value > 0.0f) {
          maxIntensity = std::max(maxIntensity, value);
          sumIncluded += value;
          numIncluded++;
        } else {
          sumExcluded += value;
          numExcluded++;
        }

        // counting the number of pixels along the normal (within the mask)
        maskCount++;
      }

      // move along the normal
      stepsTraveled++;

      if (negativeNormal) {
        // we go the opposite way for the wall
        p[0] = p[0] - normal[0];
        p[1] = p[1] - normal[1];
        p[2] = p[2] - normal[2];
      } else {
        p[0] = p[0] + normal[0];
        p[1] = p[1] + normal[1];
        p[2] = p[2] + normal[2];
      }

      maskValue = getInterpolatedValue(mWallData, p, mMaskLinearInterpolation);
      imageValue = getInterpolatedValue(mImageData, p, mDataLinearInterpolation);
    }

    if (maskCount >= 1) {
      if (mScalarMethod == ScalarMethod::NORMAL_MAXIMUM_INTENSITY) {
        returnValue = maxIntensity;
      } else if (mScalarMethod == ScalarMethod::NORMAL_PERCENTAGE) {
        returnValue = numIncluded / (numIncluded + numExcluded) * 100.0f;
      } else if (mScalarMethod == ScalarMethod::NORMAL_SUM) {
        returnValue = sumIncluded;
      } else if (mScalarMethod == ScalarMethod::NORMAL_WALL_THICKNESS) {
        // return count of pixels in the mask along normal instead
        returnValue = maskCount;
      } else if (mScalarMethod == ScalarMethod::HIT_THRESHOLD) {
        if (numIncluded > 0) {
          returnValue = 100;
        } else {
          returnValue = 0;
        }
      } else if (mScalarMethod == ScalarMethod::NORMAL_AVERAGE_POST_THRESHOLD) {
        float average = (sumIncluded + sumExcluded) / (numExcluded+numIncluded);
        returnValue = applyThreshold(average);

        // AKM: For the grey non-threshold pictures
        //if (returnValue == 0.0f) {
        //  returnValue = -1;
        //}

      }
    }

  } else { // neighborhood

    for (float xAdd = -1.0f; xAdd <= 1.0f ; xAdd++) {
      for (float yAdd = -1.0f; yAdd <= 1.0f ; yAdd++) {
        for (float zAdd = -1.0f; zAdd <= 1.0f ; zAdd++) {
          p[0] = point[0] + xAdd;
          p[1] = point[1] + xAdd;
          p[2] = point[2] + zAdd;  

          maskValue = getInterpolatedValue(mWallData, p, mMaskLinearInterpolation);
          imageValue = getInterpolatedValue(mImageData, p, mDataLinearInterpolation);

          if (maskValue >= maskCheck) {
            maskCount++;
            // maximum intensity value, but only if above threshold
            float value = applyThreshold(imageValue);
            if (value > 0.0f) {
              maxIntensity = std::max(maxIntensity, value);
              sumIncluded += value;
              numIncluded++;
            } else {
              numExcluded++;
            }
          }
        }
      }
    }

    if (maskCount >= 1) {
      if (mScalarMethod == ScalarMethod::NEIGHBORHOOD_MAXIMUM_INTENSITY) {
        returnValue = maxIntensity;
      } else if (mScalarMethod == ScalarMethod::NEIGHBORHOOD_PERCENTAGE) {
        returnValue = numIncluded / (numIncluded + numExcluded) * 100.0f;
      } else if (mScalarMethod == ScalarMethod::NEIGHBORHOOD_SUM) {
        returnValue = sumIncluded;
      } else if (mScalarMethod == ScalarMethod::NEIGHBORHOOD_WALL_THICKNESS) {
        // return count of pixels in the mask along normal instead
        returnValue = maskCount;
      }
    }
  }

  return returnValue;
}

void IsoRenderer::updateHedgeHogs() {


  mSurfaceRenderer->RemoveActor(mHedgeHogActor);

  if (mShowNormals) {


    vtkSmartPointer<vtkHedgeHog> hedgehog = vtkSmartPointer<vtkHedgeHog>::New();

    if (mScalarOnWall) {
      hedgehog->SetInput(mWallPolyDataNormals->GetOutput());
    } else {
      hedgehog->SetInput(mEndoPolyDataNormals->GetOutput());
    }
    hedgehog->SetVectorModeToUseNormal();
    hedgehog->SetScaleFactor(1.0f);

    mHedgeHogMapper->SetInputConnection(hedgehog->GetOutputPort());


    mHedgeHogActor->SetMapper(mHedgeHogMapper);
    //sgridActor->GetProperty()->SetColor(0,0,0);



    vtkLookupTable *hedgeHogLUT = vtkLookupTable::New();
    //blueGreenLUT->SetNumberOfColors(100);
    //blueGreenLUT->SetHueRange(0.66,0.33);
    //blueGreenLUT->SetHueRange(0.0,1.0);
    // 


    hedgeHogLUT->SetRange(0, 2000);
    hedgeHogLUT->SetValueRange(0.0, 1.0);
    hedgeHogLUT->SetSaturationRange(0.0, 0.0);
    hedgeHogLUT->SetRampToLinear();
    hedgeHogLUT->Build();
    mHedgeHogMapper->SetLookupTable(hedgeHogLUT);


    mSurfaceRenderer->AddActor(mHedgeHogActor);
  }

}


void IsoRenderer::setLinearInterpolation( bool image, bool mask )
{
  mDataLinearInterpolation = image;
  mMaskLinearInterpolation = mask;
}


float IsoRenderer::getInterpolatedValue( Image *image, double p[3], bool linear) 
{
  if (linear) {
    return image->getLinearInterpolatedPixel(p);
  } else {
    return image->getNearestInterpolatedPixel(p);
  }
}

void IsoRenderer::setScalarMethod( ScalarMethod method )
{
  mScalarMethod = method;
}



// use a sigmoid function to smooth out the stair step function of the threshold (in or out)
// TODO: when smoothing is on, the upper threshold is ignored!
float IsoRenderer::applyThreshold(float value) {
  if (mSmoothThreshold) {
    float x = value - mThresholdLower;

    //float width = 1;
    float width = mSmoothThresholdWidth;


    float amount = 1/(1+std::exp(-width * (x+1)));

    return value * amount;
  } else {
    if (value >= mThresholdLower && value <= mThresholdUpper) {
      return value;
    } else {
      return 0.0f;
    }
  }
}

void IsoRenderer::redrawIsoSurface()
{
  mSurfaceRenderer->Render();
}

void IsoRenderer::setSmoothThresholdWidth( float value )
{
  mSmoothThresholdWidth = value;
}

vtkLookupTable *IsoRenderer::getLookupTable()
{
  return mScalarLUT;
}

void IsoRenderer::setSliceDisplayExtent( int *extent )
{
  // update the slice actor in the 3d viewer to match.
  mSliceActor->SetDisplayExtent(extent);
}

void IsoRenderer::setSmoothSettings( SmoothSettings settings )
{
  mSmoothSettings = settings;
}

void IsoRenderer::setThreshold( float lower, float upper )
{
  this->mThresholdLower = lower;
  this->mThresholdUpper = upper;
}

float IsoRenderer::getScalarMin()
{
  return mScalarMin;
}

float IsoRenderer::getScalarMax()
{
  return mScalarMax;
}

void IsoRenderer::setScalarOnWall( bool scalarOnWall )
{
  mScalarOnWall = scalarOnWall;
}

void IsoRenderer::setSmoothThreshold( bool value )
{
  mSmoothThreshold = value;
}

void IsoRenderer::setMeshSubdivision( bool value )
{
  mSubDivideMesh = value;
}







