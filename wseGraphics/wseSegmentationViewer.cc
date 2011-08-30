#include "wseSegmentationViewer.h"

namespace wse {

vtkCxxRevisionMacro(SegmentationViewer, "$Revision: 1.2 $");
vtkStandardNewMacro(SegmentationViewer);

SegmentationViewer::SegmentationViewer()
{
}

SegmentationViewer::~SegmentationViewer()
{
}

void SegmentationViewer::InstallPipeline()
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

       // DO NOT INSTALL WINDOW / LEVELING EVENTS
       //   this->InteractorStyle->AddObserver(
       //      vtkCommand::WindowLevelEvent, cbk);
       //   this->InteractorStyle->AddObserver(
       //      vtkCommand::StartWindowLevelEvent, cbk);
       //        this->InteractorStyle->AddObserver(
       //     vtkCommand::ResetWindowLevelEvent, cbk);
        cbk->Delete();
     }
     this->Interactor->SetInteractorStyle(this->InteractorStyle);
     this->Interactor->SetRenderWindow(this->RenderWindow);
   }

  // setup mask image map using the lookup table
  MaskImageMapToColors->SetLookupTable(mMaskLUT);
  MaskImageMapToColors->PassAlphaToOutputOn();
  MaskImageMapToColors->SetOutputFormatToRGBA();

  if (mImageLookupTable != NULL)
    {
      WindowLevel->SetLookupTable(mImageLookupTable);
    }
  WindowLevel->PassAlphaToOutputOff();
  WindowLevel->SetOutputFormatToRGBA();

  //ImageActor->SetInput(mImageBlend->GetOutput());

  if (this->Renderer && this->ImageActor)
  {
    this->Renderer->AddViewProp(this->ImageActor);
    this->Renderer->AddViewProp(this->MaskImageActor);
  }
  mPipelineInstalled = true;
}

void SegmentationViewer::PrintSelf(ostream& os, vtkIndent indent)
{  this->Superclass::PrintSelf(os, indent);}


void SegmentationViewer::ResetWindowLevel(vtkImageMapToWindowLevelColors *windowLevel)
{
  if (windowLevel) 
    {
      // These settings should nullify any window / leveling.
      // From the vtk documentation for vtkImageMapToWindowLevelColors: 
      //   "modulation will be performed on the color based on 
      //    (S - (L - W/2))/W where S is the scalar
      //    value, L is the level and W is the window."
      windowLevel->SetLevel(0.5);
      windowLevel->SetWindow(1.0);
    
    if (mImageLookupTable != NULL)
      {
        windowLevel->SetLookupTable(mImageLookupTable);
      }

    }
}

} // end namespace wse
