#include "wse.h"
#include "QThread.h"

namespace wse {

void wseGUI::mITKFilteringThread_finished()
{     
  ui.progressBar->setValue(100);
  ui.progressBar->hide();
  if (mITKFilteringThread->errorFlag() == true)
    {
      QMessageBox::warning(this, tr("Filtering aborted"), mITKFilteringThread->errorString());
      this->output("Filter operation aborted by user");
    }  
  this->output("Filtering operation finished");

}
  
void wseGUI::mITKFilteringThread_started()
{ 
  // Hook up to itk progress event
  itk::wseITKCallback<wseGUI>::Pointer mycmd = itk::wseITKCallback<wseGUI>::New();
  // wseITKCallback *mycmd = new wseITKCallback();
  mycmd->setGui(this);
    
  //    cmd->SetCallback(&mITKFilteringThread_progress);
  mITKFilteringThread->filter()->AddObserver(itk::ProgressEvent(), mycmd);
  
  ui.progressBar->setValue(0);
  ui.progressBar->show();
}
  
void wseGUI::mITKFilteringThread_progress(itk::Object *caller, const itk::EventObject& event)
{
  itk::ProcessObject *processObject = (itk::ProcessObject*)caller;
  if (typeid(event) == typeid(itk::ProgressEvent)) 
    {
      //  std::cout << "ITK Progress event received from "
      //		<< processObject->GetNameOfClass() << ". Progress is "
      //		<< 100.0 * processObject->GetProgress() << " %."
      //	<< std::endl;
      ui.progressBar->setValue(100.0 * processObject->GetProgress());
    }
}
  

void wseGUI::runGaussianFiltering()
{
  this->output(QString("Starting Gaussian filtering with sigma %1").arg(ui.smoothingSigmaInputBox->value()));

  itk::DiscreteGaussianImageFilter<itkFloatImage,itkFloatImage>::Pointer filter
    =  itk::DiscreteGaussianImageFilter<itkFloatImage,itkFloatImage>::New();
  filter->SetInput(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->original());
  filter->SetVariance(ui.smoothingSigmaInputBox->value() * ui.smoothingSigmaInputBox->value());
  filter->SetUseImageSpacingOff();
  
  // MULTITHREADING:
  mITKFilteringThread->setFilter(filter);
  mITKFilteringThread->start();
  
  Image *img = new Image(filter->GetOutput());
  img->name(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->name() + QString(" (gaussian blur)"));
  this->addImageFromData(img);
}

void wseGUI::runAnisotropicFiltering()
{
  this->output(QString("Starting %1 iterations of Classic Anisotropic Diffusion filtering with conductance of %2").arg(ui.conductanceSpinBox->value()).arg(ui.iterationsSpinBox->value()));

  itk::GradientAnisotropicDiffusionImageFilter<itkFloatImage,itkFloatImage>::Pointer filter
    =  itk::GradientAnisotropicDiffusionImageFilter<itkFloatImage,itkFloatImage>::New();
  filter->SetInput(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->original());
  filter->SetConductanceParameter(ui.conductanceSpinBox->value());
  filter->SetTimeStep(0.062);
  filter->SetNumberOfIterations(ui.iterationsSpinBox->value());
  
  // MULTITHREADING:
  mITKFilteringThread->setFilter(filter);
  mITKFilteringThread->start();
  
  Image *img = new Image(filter->GetOutput());
  img->name(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->name() + QString(" (classic anisotropic)"));
  this->addImageFromData(img);
}


void wseGUI::runCurvatureFiltering()
{

  this->output(QString("Starting %1 iterations of Curvature Anisotropic Diffusion filtering with conductance of %2").arg(ui.conductanceSpinBox->value()).arg(ui.iterationsSpinBox->value()));

  itk::CurvatureAnisotropicDiffusionImageFilter<itkFloatImage,itkFloatImage>::Pointer filter
    =  itk::CurvatureAnisotropicDiffusionImageFilter<itkFloatImage,itkFloatImage>::New();
  filter->SetInput(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->original());
  filter->SetConductanceParameter(ui.conductanceSpinBox->value());
  filter->SetTimeStep(0.062);
  filter->SetNumberOfIterations(ui.iterationsSpinBox->value());
  
  // MULTITHREADING:
  mITKFilteringThread->setFilter(filter);
  mITKFilteringThread->start();
  
  Image *img = new Image(filter->GetOutput());
  img->name(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->name() + QString(" (curvature anisotropic)"));
  this->addImageFromData(img);
}

void wseGUI::runGradientFiltering()
{
  this->output(QString("Running the gradient magnitude filter."));

  itk::GradientMagnitudeImageFilter<itkFloatImage, itkFloatImage>::Pointer filter = 
    itk::GradientMagnitudeImageFilter<itkFloatImage, itkFloatImage>::New();
  filter->SetInput(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->original());
  filter->SetUseImageSpacingOff();
  
  // MULTITHREADING:
  mITKFilteringThread->setFilter(filter);
  mITKFilteringThread->start();
  
  Image *img = new Image(filter->GetOutput());
  img->name(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->name() + QString(" (grad. magnitude)"));
  this->addImageFromData(img);
}




} //end namespace wse
