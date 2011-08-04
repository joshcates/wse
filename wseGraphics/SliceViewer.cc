#include "SliceViewer.h"

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: SliceViewer.cc,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkImageData.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkInteractorStyleImage.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkImageMapper.h"
#include "vtkLookupTable.h"
#include "vtkAlgorithmOutput.h"
#include "vtkImageBlend.h"
#include "vtkImageThreshold.h"
#include "vtkImageStencil.h"
#include "vtkImageToImageStencil.h"
#include "vtkImageMask.h"
#include "vtkImageFlip.h"

vtkCxxRevisionMacro(SliceViewer, "$Revision: 1.2 $");
vtkStandardNewMacro(SliceViewer);

//----------------------------------------------------------------------------
SliceViewer::SliceViewer()
{
  this->RenderWindow    = NULL;
  this->Renderer        = NULL;
  this->ImageActor      = vtkImageActor::New();
  this->WindowLevel     = vtkImageMapToWindowLevelColors::New();
  this->MaskImageActor      = vtkImageActor::New();
  this->MaskWindowLevel     = vtkImageMapToWindowLevelColors::New();
  this->MaskImageMapToColors = vtkImageMapToColors::New();
  this->mImageBlend = vtkImageBlend::New();
  this->Interactor      = NULL;
  this->InteractorStyle = NULL;
  this->mThresholdLower = 160.0f;
  this->mShowMask = true;
  this->mMaskOpacity = 0.50f;
  this->mThresholdOpacity = 0.75f;
  this->Slice = 0;
  this->FirstRender = 1;
  this->SliceOrientation = SliceViewer::SLICE_ORIENTATION_XY;
  this->mShowThreshold = true;
  this->mClipThresholdToMask = true;
  this->mPipelineInstalled = false;

  mImageThreshold = vtkImageThreshold::New();
  mFinalBlend = vtkImageBlend::New();
  mAlphaMap = vtkImageMapToColors::New();
  mAlphaLUT = vtkLookupTable::New();
  mStencilBlend = vtkImageBlend::New();
  mStencilLUT = vtkLookupTable::New();
  mThresholdImageMapToColors = vtkImageMapToColors::New();
  mThresholdLUT = vtkLookupTable::New();
  mStencilLUT = vtkLookupTable::New();
  mStencilMap = vtkImageMapToColors::New();
  mAlphaLUT = vtkLookupTable::New();
  //mImageFlip = vtkImageFlip::New();


  // create lookup table for mask, currently red with the const mMaskOpacity
  mMaskLUT = vtkLookupTable::New();
  mMaskLUT->SetRange(0,1);
  mMaskLUT->SetNumberOfColors(2);
  mMaskLUT->Build();
  mMaskLUT->SetTableValue(0,0,0,0,0.0);
  mMaskLUT->SetTableValue(1,1,0,0,mMaskOpacity);


  this->mImage = NULL;
  this->mMask = NULL;

  // Setup the pipeline

  vtkRenderWindow *renwin = vtkRenderWindow::New();
  this->SetRenderWindow(renwin);
  renwin->Delete();

  vtkRenderer *ren = vtkRenderer::New();
  this->SetRenderer(ren);
  ren->Delete();

  //  MaskWindowLevel->SetInput(vtkImageData::New());
  //  WindowLevel->SetInput(vtkImageData::New());

  this->InstallPipeline();
}

//----------------------------------------------------------------------------
SliceViewer::~SliceViewer()
{
  if (this->WindowLevel)
  {
    this->WindowLevel->Delete();
    this->WindowLevel = NULL;
  }

  if (this->ImageActor)
  {
    this->ImageActor->Delete();
    this->ImageActor = NULL;
  }

  if (this->Renderer)
  {
    this->Renderer->Delete();
    this->Renderer = NULL;
  }

  if (this->RenderWindow)
  {
    this->RenderWindow->Delete();
    this->RenderWindow = NULL;
  }

  if (this->Interactor)
  {
    this->Interactor->Delete();
    this->Interactor = NULL;
  }

  if (this->InteractorStyle)
  {
    this->InteractorStyle->Delete();
    this->InteractorStyle = NULL;
  }
}

//----------------------------------------------------------------------------
void SliceViewer::SetupInteractor(vtkRenderWindowInteractor *arg)
{
  if (this->Interactor == arg)
  {
    return;
  }

  this->UnInstallPipeline();

  if (this->Interactor)
  {
    this->Interactor->UnRegister(this);
  }

  this->Interactor = arg;

  if (this->Interactor)
  {
    this->Interactor->Register(this);
  }

  this->InstallPipeline();

  if (this->Renderer)
  {
    this->Renderer->GetActiveCamera()->ParallelProjectionOn();
  }
}

//----------------------------------------------------------------------------
void SliceViewer::SetRenderWindow(vtkRenderWindow *arg)
{
  if (this->RenderWindow == arg)
  {
    return;
  }

  this->UnInstallPipeline();

  if (this->RenderWindow)
  {
    this->RenderWindow->UnRegister(this);
  }

  this->RenderWindow = arg;

  if (this->RenderWindow)
  {
    this->RenderWindow->Register(this);
  }

  this->InstallPipeline();
}

//----------------------------------------------------------------------------
void SliceViewer::SetRenderer(vtkRenderer *arg)
{
  if (this->Renderer == arg)
  {
    return;
  }

  this->UnInstallPipeline();

  if (this->Renderer)
  {
    this->Renderer->UnRegister(this);
  }

  this->Renderer = arg;

  if (this->Renderer)
  {
    this->Renderer->Register(this);
  }

  this->InstallPipeline();
  this->UpdateOrientation();
}

//----------------------------------------------------------------------------
void SliceViewer::SetSize(int a,int b)
{
  this->RenderWindow->SetSize(a, b);
}

//----------------------------------------------------------------------------
int* SliceViewer::GetSize()
{
  return this->RenderWindow->GetSize();
}

//----------------------------------------------------------------------------
void SliceViewer::GetSliceRange(int &min, int &max)
{
  vtkImageData *input = this->GetInput();
  if (input)
  {
    input->UpdateInformation();
    int *w_ext = input->GetWholeExtent();
    min = w_ext[this->SliceOrientation * 2];
    max = w_ext[this->SliceOrientation * 2 + 1];
  }
}

//----------------------------------------------------------------------------
int* SliceViewer::GetSliceRange()
{
  vtkImageData *input = this->GetInput();
  if (input)
  {
    input->UpdateInformation();
    return input->GetWholeExtent() + this->SliceOrientation * 2;
  }
  return NULL;
}

//----------------------------------------------------------------------------
int SliceViewer::GetSliceMin()
{
  int *range = this->GetSliceRange();
  if (range)
  {
    return range[0];
  }
  return 0;
}

//----------------------------------------------------------------------------
int SliceViewer::GetSliceMax()
{
  int *range = this->GetSliceRange();
  if (range)
  {
    return range[1];
  }
  return 0;
}

//----------------------------------------------------------------------------
void SliceViewer::SetSlice(int slice)
{
  int *range = this->GetSliceRange();
  if (range)
  {
    if (slice < range[0])
    {
      slice = range[0];
    }
    else if (slice > range[1])
    {
      slice = range[1];
    }
  }

  if (this->Slice == slice)
  {
    return;
  }

  this->Slice = slice;
  this->Modified();

  this->UpdateDisplayExtent();
  this->Render();
}

//----------------------------------------------------------------------------
void SliceViewer::SetSliceOrientation(int orientation)
{
  if (orientation < SliceViewer::SLICE_ORIENTATION_YZ ||
      orientation > SliceViewer::SLICE_ORIENTATION_XY)
  {
    vtkErrorMacro("Error - invalid slice orientation " << orientation);
    return;
  }

  if (this->SliceOrientation == orientation)
  {
    return;
  }

  this->SliceOrientation = orientation;

  // Update the viewer

  int *range = this->GetSliceRange();
  if (range)
  {
    this->Slice = static_cast<int>((range[0] + range[1]) * 0.5);
  }

  this->UpdateOrientation();
  this->UpdateDisplayExtent();

  if (this->Renderer && this->GetInput())
  {
    double scale = this->Renderer->GetActiveCamera()->GetParallelScale();
    this->Renderer->ResetCamera();
    this->Renderer->GetActiveCamera()->SetParallelScale(scale);
  }

  this->Render();
}

//----------------------------------------------------------------------------
void SliceViewer::UpdateOrientation()
{
  // Set the camera position

  vtkCamera *cam = this->Renderer ? this->Renderer->GetActiveCamera() : NULL;
  if (cam)
  {
    switch (this->SliceOrientation)
    {
    case SliceViewer::SLICE_ORIENTATION_XY:
      cam->SetFocalPoint(0,0,0);
      cam->SetPosition(0,0,1); // -1 if medical ?
      cam->SetViewUp(0,1,0);
      break;

    case SliceViewer::SLICE_ORIENTATION_XZ:
      cam->SetFocalPoint(0,0,0);
      cam->SetPosition(0,-1,0); // 1 if medical ?
      cam->SetViewUp(0,0,1);
      break;

    case SliceViewer::SLICE_ORIENTATION_YZ:
      cam->SetFocalPoint(0,0,0);
      cam->SetPosition(1,0,0); // -1 if medical ?
      cam->SetViewUp(0,0,1);
      break;
    }
  }
}

//----------------------------------------------------------------------------
void SliceViewer::UpdateDisplayExtent()
{
  vtkImageData *input = this->GetInput();
  if (!input || !this->ImageActor)
  {
    return;
  }

  input->UpdateInformation();
  int *w_ext = input->GetWholeExtent();

  // Is the slice in range ? If not, fix it

  int slice_min = w_ext[this->SliceOrientation * 2];
  int slice_max = w_ext[this->SliceOrientation * 2 + 1];
  if (this->Slice < slice_min || this->Slice > slice_max)
  {
    this->Slice = static_cast<int>((slice_min + slice_max) * 0.5);
  }

  // Set the image actor

  switch (this->SliceOrientation)
  {
  case SliceViewer::SLICE_ORIENTATION_XY:
    this->ImageActor->SetDisplayExtent(
        w_ext[0], w_ext[1], w_ext[2], w_ext[3], this->Slice, this->Slice);
    this->MaskImageActor->SetDisplayExtent(
        w_ext[0], w_ext[1], w_ext[2], w_ext[3], this->Slice, this->Slice);
    break;

  case SliceViewer::SLICE_ORIENTATION_XZ:
    this->ImageActor->SetDisplayExtent(
        w_ext[0], w_ext[1], this->Slice, this->Slice, w_ext[4], w_ext[5]);
    this->MaskImageActor->SetDisplayExtent(
        w_ext[0], w_ext[1], this->Slice, this->Slice, w_ext[4], w_ext[5]);
    break;

  case SliceViewer::SLICE_ORIENTATION_YZ:
    this->ImageActor->SetDisplayExtent(
        this->Slice, this->Slice, w_ext[2], w_ext[3], w_ext[4], w_ext[5]);
    this->MaskImageActor->SetDisplayExtent(
        this->Slice, this->Slice, w_ext[2], w_ext[3], w_ext[4], w_ext[5]);
    break;
  }

  // Figure out the correct clipping range

  if (this->Renderer)
  {
    if (this->InteractorStyle &&
        this->InteractorStyle->GetAutoAdjustCameraClippingRange())
    {
      this->Renderer->ResetCameraClippingRange();
    }
    else
    {
      vtkCamera *cam = this->Renderer->GetActiveCamera();
      if (cam)
      {
        double bounds[6];
        this->ImageActor->GetBounds(bounds);
        double spos = bounds[this->SliceOrientation * 2];
        double cpos = cam->GetPosition()[this->SliceOrientation];
        double range = fabs(spos - cpos);
        double *spacing = input->GetSpacing();
        double avg_spacing =
            (spacing[0] + spacing[1] + spacing[2]) / 3.0;
        cam->SetClippingRange(
            range - avg_spacing * 3.0, range + avg_spacing * 3.0);
      }
    }
  }
}

//----------------------------------------------------------------------------
void SliceViewer::SetPosition(int a,int b)
{
  this->RenderWindow->SetPosition(a, b);
}

//----------------------------------------------------------------------------
int* SliceViewer::GetPosition()
{
  return this->RenderWindow->GetPosition();
}

//----------------------------------------------------------------------------
void SliceViewer::SetDisplayId(void *a)
{
  this->RenderWindow->SetDisplayId(a);
}

//----------------------------------------------------------------------------
void SliceViewer::SetWindowId(void *a)
{
  this->RenderWindow->SetWindowId(a);
}

//----------------------------------------------------------------------------
void SliceViewer::SetParentId(void *a)
{
  this->RenderWindow->SetParentId(a);
}

//----------------------------------------------------------------------------
double SliceViewer::GetColorWindow()
{
  return this->WindowLevel->GetWindow();
}

//----------------------------------------------------------------------------
double SliceViewer::GetColorLevel()
{
  return this->WindowLevel->GetLevel();
}

//----------------------------------------------------------------------------
void SliceViewer::SetColorWindow(double s)
{
  this->WindowLevel->SetWindow(s);
}

//----------------------------------------------------------------------------
void SliceViewer::SetColorLevel(double s)
{
  this->WindowLevel->SetLevel(s);
}

//----------------------------------------------------------------------------
class imageViewerCallback : public vtkCommand
{
public:
  static imageViewerCallback *New() { return new imageViewerCallback; }

  void Execute(vtkObject *caller,
               unsigned long event,
               void *vtkNotUsed(callData))
  {
    if (this->IV->GetInput() == NULL)
    {
      return;
    }

    // Reset

    if (event == vtkCommand::ResetWindowLevelEvent)
    {
      this->IV->GetInput()->UpdateInformation();
      this->IV->GetInput()->SetUpdateExtent
          (this->IV->GetInput()->GetWholeExtent());
      this->IV->GetInput()->Update();
      double *range = this->IV->GetInput()->GetScalarRange();
      this->IV->SetColorWindow(range[1] - range[0]);
      this->IV->SetColorLevel(0.5 * (range[1] + range[0]));
      this->IV->Render();
      return;
    }

    // Start

    if (event == vtkCommand::StartWindowLevelEvent)
    {
      this->InitialWindow = this->IV->GetColorWindow();
      this->InitialLevel = this->IV->GetColorLevel();
      return;
    }

    // Adjust the window level here

    vtkInteractorStyleImage *isi =
        static_cast<vtkInteractorStyleImage *>(caller);

    int *size = this->IV->GetRenderWindow()->GetSize();
    double window = this->InitialWindow;
    double level = this->InitialLevel;

    // Compute normalized delta

    double dx = 4.0 *
                (isi->GetWindowLevelCurrentPosition()[0] -
                 isi->GetWindowLevelStartPosition()[0]) / size[0];
    double dy = 4.0 *
                (isi->GetWindowLevelStartPosition()[1] -
                 isi->GetWindowLevelCurrentPosition()[1]) / size[1];

    // Scale by current values

    if (fabs(window) > 0.01)
    {
      dx = dx * window;
    }
    else
    {
      dx = dx * (window < 0 ? -0.01 : 0.01);
    }
    if (fabs(level) > 0.01)
    {
      dy = dy * level;
    }
    else
    {
      dy = dy * (level < 0 ? -0.01 : 0.01);
    }

    // Abs so that direction does not flip

    if (window < 0.0)
    {
      dx = -1*dx;
    }
    if (level < 0.0)
    {
      dy = -1*dy;
    }

    // Compute new window level

    double newWindow = dx + window;
    double newLevel;
    newLevel = level - dy;

    // Stay away from zero and really

    if (fabs(newWindow) < 0.01)
    {
      newWindow = 0.01*(newWindow < 0 ? -1 : 1);
    }
    if (fabs(newLevel) < 0.01)
    {
      newLevel = 0.01*(newLevel < 0 ? -1 : 1);
    }

    this->IV->SetColorWindow(newWindow);
    this->IV->SetColorLevel(newLevel);
    this->IV->Render();
  }

  SliceViewer *IV;
  double InitialWindow;
  double InitialLevel;
};

//----------------------------------------------------------------------------
void SliceViewer::InstallPipeline()
{
  if (this->RenderWindow && this->Renderer)
  {
    this->RenderWindow->AddRenderer(this->Renderer);
  }

  if (this->Interactor)
  {
    if (!this->InteractorStyle)
    {
      this->InteractorStyle = vtkInteractorStyleImage::New();
      imageViewerCallback *cbk = imageViewerCallback::New();
      cbk->IV = this;
      this->InteractorStyle->AddObserver(
          vtkCommand::WindowLevelEvent, cbk);
      this->InteractorStyle->AddObserver(
          vtkCommand::StartWindowLevelEvent, cbk);
      this->InteractorStyle->AddObserver(
          vtkCommand::ResetWindowLevelEvent, cbk);
      cbk->Delete();
    }

    this->Interactor->SetInteractorStyle(this->InteractorStyle);
    this->Interactor->SetRenderWindow(this->RenderWindow);
  }


  // setup mask image map using the lookup table
  MaskImageMapToColors->SetLookupTable(mMaskLUT);
  MaskImageMapToColors->PassAlphaToOutputOn();
  MaskImageMapToColors->SetOutputFormatToRGBA();

  WindowLevel->PassAlphaToOutputOn();
  WindowLevel->SetOutputFormatToRGBA();

  //ImageActor->SetInput(mImageBlend->GetOutput());

  if (this->Renderer && this->ImageActor)
  {
    this->Renderer->AddViewProp(this->ImageActor);
    this->Renderer->AddViewProp(this->MaskImageActor);
  }
  mPipelineInstalled = true;


}

//----------------------------------------------------------------------------
void SliceViewer::UnInstallPipeline()
{
  if (this->ImageActor)
  {
    this->ImageActor->SetInput(NULL);
  }

  if (this->MaskImageActor)
  {
    this->MaskImageActor->SetInput(NULL);
  }

  if (this->Renderer && this->ImageActor)
  {
    this->Renderer->RemoveViewProp(this->ImageActor);
    this->Renderer->RemoveViewProp(this->MaskImageActor);
  }

  if (this->RenderWindow && this->Renderer)
  {
    this->RenderWindow->RemoveRenderer(this->Renderer);
  }

  if (this->Interactor)
  {
    this->Interactor->SetInteractorStyle(NULL);
    this->Interactor->SetRenderWindow(NULL);
  }
  
  this->SetInput(NULL);
  mImageBlend->RemoveAllInputs();

  this->mImage = NULL;
  this->mMask = NULL;
  mPipelineInstalled = false;
}

//----------------------------------------------------------------------------
void SliceViewer::Render()
{
  if (this->FirstRender)
  {

    // Initialize the size if not set yet

    vtkImageData *input = this->GetInput();
    if (input)
    {
      input->UpdateInformation();
      int *w_ext = input->GetWholeExtent();
      int xs = 0, ys = 0;

      switch (this->SliceOrientation)
      {
      case SliceViewer::SLICE_ORIENTATION_XY:
      default:
        xs = w_ext[1] - w_ext[0] + 1;
        ys = w_ext[3] - w_ext[2] + 1;
        break;

      case SliceViewer::SLICE_ORIENTATION_XZ:
        xs = w_ext[1] - w_ext[0] + 1;
        ys = w_ext[5] - w_ext[4] + 1;
        break;

      case SliceViewer::SLICE_ORIENTATION_YZ:
        xs = w_ext[3] - w_ext[2] + 1;
        ys = w_ext[5] - w_ext[4] + 1;
        break;
      }

      // if it would be smaller than 150 by 100 then limit to 150 by 100
      if (this->RenderWindow->GetSize()[0] == 0)
      {
        this->RenderWindow->SetSize(
            xs < 150 ? 150 : xs, ys < 100 ? 100 : ys);
      }

      if (this->Renderer)
      {
        this->Renderer->ResetCamera();
        //        this->Renderer->GetActiveCamera()->SetParallelScale(
        //          xs < 150 ? 75 : (xs - 1 ) / 2.0);
        //      AKM: I've changed the default zoom so that it more closely matches what we had from vtkImageViewer
        this->Renderer->GetActiveCamera()->SetParallelScale(
            xs < 150 ? 75 : (xs - 1 ) / 2.5);


        this->Renderer->GetActiveCamera()->Azimuth(180);
        this->Renderer->GetActiveCamera()->Roll(180);


      }
      this->FirstRender = 0;
    }
  }
  if (this->GetInput())
  {
    this->RenderWindow->Render();
  }
}

//----------------------------------------------------------------------------
const char* SliceViewer::GetWindowName()
{
  return this->RenderWindow->GetWindowName();
}

//----------------------------------------------------------------------------
void SliceViewer::SetOffScreenRendering(int i)
{
  this->RenderWindow->SetOffScreenRendering(i);
}

//----------------------------------------------------------------------------
int SliceViewer::GetOffScreenRendering()
{
  return this->RenderWindow->GetOffScreenRendering();
}

//----------------------------------------------------------------------------
void SliceViewer::SetInput(vtkImageData *in)
{
  this->WindowLevel->SetInput(in);
  this->UpdateDisplayExtent();
}
//----------------------------------------------------------------------------
vtkImageData* SliceViewer::GetInput()
{
  return vtkImageData::SafeDownCast(this->WindowLevel->GetInput());
}



//----------------------------------------------------------------------------
void SliceViewer::SetInputConnection(vtkAlgorithmOutput* input)
{
  if (mPipelineInstalled == false) {
    InstallPipeline();
  }
  if (mImage == input) {
    return;
  }

  mImage = input;
  this->UpdateDisplay(); 
  ResetWindowLevel(WindowLevel);
  this->UpdateDisplayExtent();
}


void SliceViewer::DisableDisplay() {
  this->UnInstallPipeline();
}


void SliceViewer::UpdateDisplay()
{
  if (mPipelineInstalled == false) {
    return;
  }
  mImageBlend->RemoveAllInputs();

  if (mImage) {
    this->WindowLevel->SetInputConnection(mImage);
    mImageBlend->AddInputConnection(WindowLevel->GetOutputPort());
  }

  if (mMask && mShowMask) {
    this->MaskImageMapToColors->SetInputConnection(mMask);
    mImageBlend->AddInputConnection(MaskImageMapToColors->GetOutputPort());
  }

  if (mImage && mMask && mShowThreshold) {
    // Use a vtkImageThreshold to apply the histogram threshold to the original image
    mImageThreshold->RemoveAllInputs();
    mImageThreshold->SetInputConnection(mImage);
    //mImageThreshold->ThresholdByUpper(mThresholdLower);
    mImageThreshold->ThresholdBetween(mThresholdLower, mThresholdUpper);
    mImageThreshold->ReplaceInOn();
    mImageThreshold->ReplaceOutOn();
    mImageThreshold->SetInValue(0);
    mImageThreshold->SetOutValue(1);
    mImageThreshold->Update();

    // Threshold image table sets alpha values depending on the clipping
    mThresholdLUT->SetRange(0,1);
    mThresholdLUT->SetNumberOfColors(2);
    mThresholdLUT->Build();
    if (mClipThresholdToMask) {
      mThresholdLUT->SetTableValue(0,1,1,1,1);
    } else {
      mThresholdLUT->SetTableValue(0,0,1,0,1);
    }
    mThresholdLUT->SetTableValue(1,0,0,0,0 /*opacity*/);

    // vtkImageMapToColors applies the mThresholdLUT to the mImageThreshold
    mThresholdImageMapToColors->RemoveAllInputs();
    mThresholdImageMapToColors->SetLookupTable(mThresholdLUT);
    mThresholdImageMapToColors->PassAlphaToOutputOn();
    mThresholdImageMapToColors->SetOutputFormatToRGBA();
    mThresholdImageMapToColors->SetInputConnection(mImageThreshold->GetOutputPort());

    // create stencil LUT for the mask
    mStencilLUT->SetRange(0,1);
    mStencilLUT->SetNumberOfColors(2);
    mStencilLUT->Build();
    mStencilLUT->SetTableValue(0,0,0,0,0);
    mStencilLUT->SetTableValue(1,0,0,0,1);

    // vtkImageMapToColors for the mask
    mStencilMap->RemoveAllInputs();
    mStencilMap->SetInputConnection(mMask);
    mStencilMap->SetLookupTable(mStencilLUT);
    mStencilMap->PassAlphaToOutputOn();
    mStencilMap->SetOutputFormatToRGBA();

    // a vtkImageBlend to clip the threshold image against the mask
    mStencilBlend->RemoveAllInputs();
    mStencilBlend->AddInputConnection(mStencilMap->GetOutputPort());
    mStencilBlend->AddInputConnection(mThresholdImageMapToColors->GetOutputPort());

    // a vtkLookupTable to apply transparency for the clipped threshold
    mAlphaLUT->SetRange(0,1);
    mAlphaLUT->SetNumberOfColors(2);
    mAlphaLUT->Build();
    mAlphaLUT->SetTableValue(0,0,0,0,0);
    mAlphaLUT->SetTableValue(1,0,1,0,1);

    // a vtkImageMapToColors to apply the lookuptable to the clipped threshold
    mAlphaMap->RemoveAllInputs();
    mAlphaMap->SetInputConnection(mStencilBlend->GetOutputPort());
    mAlphaMap->SetLookupTable(mAlphaLUT);
    mAlphaMap->PassAlphaToOutputOn();
    mAlphaMap->SetOutputFormatToRGBA();

    // the final vtkImageBlend merging the original image (plus optional mask) with the threshold
    mFinalBlend->RemoveAllInputs();
    mFinalBlend->AddInputConnection(mImageBlend->GetOutputPort());

    if (mClipThresholdToMask) {
      mFinalBlend->AddInputConnection(mAlphaMap->GetOutputPort());
    } else {
      mFinalBlend->AddInputConnection(mThresholdImageMapToColors->GetOutputPort());
    }

    mFinalBlend->SetOpacity(1,mThresholdOpacity);

    //finalBlend->AddInputConnection(stencilBlend->GetOutputPort());

    //ImageActor->SetInput(imageMapToColors->GetOutput());
    //ImageActor->SetInput(mStencilBlend->GetOutput());


    mFinalOutput = mFinalBlend->GetOutputPort();
    ImageActor->SetInput(mFinalBlend->GetOutput());

  } else {

    if (mImage || mMask) {
      mFinalOutput = mImageBlend->GetOutputPort();
      ImageActor->SetInput(mImageBlend->GetOutput());
    }
  }

  // disable interpolation
  ImageActor->SetInterpolate(0);

  //mImageFlip->SetInputConnection(mFinalOutput);
  //mImageFlip->SetFilteredAxis(0);

  //ImageActor->SetInput(mImageFlip->GetOutput());
}

//----------------------------------------------------------------------------
void SliceViewer::SetImageMask(vtkAlgorithmOutput* mask)
{
  mMask = mask;
  UpdateDisplay();
  this->UpdateDisplayExtent();
}


//----------------------------------------------------------------------------
#ifndef VTK_LEGACY_REMOVE
int SliceViewer::GetWholeZMin()
{
  //  VTK_LEGACY_REPLACED_BODY(imageViewer::GetWholeZMin, "VTK 5.0",
  //                           imageViewer::GetSliceMin);
  return this->GetSliceMin();
}
int SliceViewer::GetWholeZMax()
{
  //  VTK_LEGACY_REPLACED_BODY(imageViewer::GetWholeZMax, "VTK 5.0",
  //                           imageViewer::GetSliceMax);
  return this->GetSliceMax();
}
int SliceViewer::GetZSlice()
{
  //  VTK_LEGACY_REPLACED_BODY(imageViewer::GetZSlice, "VTK 5.0",
  //                           imageViewer::GetSlice);
  return this->GetSlice();
}
void SliceViewer::SetZSlice(int s)
{
  //  VTK_LEGACY_REPLACED_BODY(imageViewer::SetZSlice, "VTK 5.0",
  //                           imageViewer::SetSlice);
  this->SetSlice(s);
}
#endif

//----------------------------------------------------------------------------
void SliceViewer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "RenderWindow:\n";
  this->RenderWindow->PrintSelf(os,indent.GetNextIndent());
  os << indent << "Renderer:\n";
  this->Renderer->PrintSelf(os,indent.GetNextIndent());
  os << indent << "ImageActor:\n";
  this->ImageActor->PrintSelf(os,indent.GetNextIndent());
  os << indent << "WindowLevel:\n" << endl;
  this->WindowLevel->PrintSelf(os,indent.GetNextIndent());
  os << indent << "Slice: " << this->Slice << endl;
  os << indent << "SliceOrientation: " << this->SliceOrientation << endl;
  os << indent << "InteractorStyle: " << endl;
  if (this->InteractorStyle)
  {
    os << "\n";
    this->InteractorStyle->PrintSelf(os,indent.GetNextIndent());
  }
  else
  {
    os << "None";
  }
}


void SliceViewer::ResetWindowLevel(vtkImageMapToWindowLevelColors *windowLevel)
{
  if (windowLevel) {
    vtkImageData *data = vtkImageData::SafeDownCast(windowLevel->GetInput());
    double *range = data->GetScalarRange();
    windowLevel->SetWindow(range[1] - range[0]);
    windowLevel->SetLevel(0.5 * (range[1] + range[0]));
  }
}

void SliceViewer::SetShowMask(bool showMask) {
  this->mShowMask = showMask;
  this->UpdateDisplay();
}

void SliceViewer::SetShowThreshold(bool showThreshold) {
  this->mShowThreshold = showThreshold;
  this->UpdateDisplay();
}

void SliceViewer::SetMaskOpacity(float opacity) {
  this->mMaskOpacity = opacity;
  mMaskLUT->SetTableValue(1,1,0,0,mMaskOpacity);
}

void SliceViewer::SetThresholdOpacity(float opacity) {
  this->mThresholdOpacity = opacity;
  this->UpdateDisplay();
}

void SliceViewer::SetThreshold(float lower, float upper) {
  mThresholdLower = lower;
  mThresholdUpper = upper;
  this->UpdateDisplay();
}

void SliceViewer::SetClipThresholdToMask(bool value) {
  mClipThresholdToMask = value;
  this->UpdateDisplay();
}


vtkAlgorithmOutput *SliceViewer:: GetFinalOutput() {
  return mFinalOutput;
}
