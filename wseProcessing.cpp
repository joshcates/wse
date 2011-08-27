#include "wse.h"

namespace wse {

void wseGUI::mITKFilteringThread_started()
{ 
  // Hook up to itk progress event
  itk::wseITKCallback<wseGUI>::Pointer mycmd = itk::wseITKCallback<wseGUI>::New();
  mycmd->setGui(this);
  mITKFilteringThread->filter()->AddObserver(itk::ProgressEvent(), mycmd);
  
  // Show progress bar
  ui.progressBar->setValue(0);
  ui.progressBar->show();
}

void wseGUI::mITKFilteringThread_finished()
{
  ui.progressBar->setValue(100);
  ui.progressBar->hide();
  if (mITKFilteringThread->errorFlag() == true)
    {
      QMessageBox::warning(this, tr("Filtering aborted"), mITKFilteringThread->errorString());
      this->output("Filter operation aborted by user");
      return;
    }  
  this->output("Filtering operation finished");

  FloatImage *img = new FloatImage(mITKFilteringThread->filter()->GetOutput());
  img->name(mITKFilteringThread->description());
  this->addImageFromData(img);

  // Switch view to the last image loaded
  this->on_setImageDataButton_released();
}
  

void wseGUI::runGaussianFiltering()
{
  this->output(QString("Starting Gaussian filtering with sigma %1").arg(ui.smoothingSigmaInputBox->value()));

  itk::DiscreteGaussianImageFilter<FloatImage::itkImageType,FloatImage::itkImageType>::Pointer filter
    =  itk::DiscreteGaussianImageFilter<FloatImage::itkImageType,FloatImage::itkImageType>::New();
  filter->SetInput(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->itkImage());
  filter->SetVariance(ui.smoothingSigmaInputBox->value() * ui.smoothingSigmaInputBox->value());
  filter->SetUseImageSpacingOff();
  
  // MULTITHREADING: Pass the filter object and a description to the
  // QThread object.  The description will be used later to create GUI
  // menu entries for the output of the filtering.
  mITKFilteringThread->setFilter(filter);
  mITKFilteringThread->setDescription(mImageStack->name(ui.denoisingInputComboBox->currentIndex()) + QString(" (gaussian)"));
  mITKFilteringThread->start();
}

void wseGUI::runAnisotropicFiltering()
{
  this->output(QString("Starting %1 iterations of Classic Anisotropic Diffusion filtering with conductance of %2")
               .arg(ui.conductanceSpinBox->value())
               .arg(ui.iterationsSpinBox->value()));

  itk::GradientAnisotropicDiffusionImageFilter<FloatImage::itkImageType,FloatImage::itkImageType>::Pointer filter
    =  itk::GradientAnisotropicDiffusionImageFilter<FloatImage::itkImageType,FloatImage::itkImageType>::New();
  filter->SetInput(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->itkImage());
  filter->SetConductanceParameter(ui.conductanceSpinBox->value());
  filter->SetTimeStep(0.062);
  filter->SetNumberOfIterations(ui.iterationsSpinBox->value());
  
  // MULTITHREADING: Pass the filter object and a description to the
  // QThread object.  The description will be used later to create GUI
  // menu entries for the output of the filtering.
  mITKFilteringThread->setFilter(filter);
  mITKFilteringThread->setDescription(mImageStack->name(ui.denoisingInputComboBox->currentIndex()) 
                                      + QString(" (classic anisotropic)"));
  mITKFilteringThread->start();
}

void wseGUI::runCurvatureFiltering()
{
  this->output(QString("Starting %1 iterations of Curvature Anisotropic Diffusion filtering with conductance of %2")
               .arg(ui.conductanceSpinBox->value())
               .arg(ui.iterationsSpinBox->value()));

  itk::CurvatureAnisotropicDiffusionImageFilter<FloatImage::itkImageType,FloatImage::itkImageType>::Pointer filter
    =  itk::CurvatureAnisotropicDiffusionImageFilter<FloatImage::itkImageType,FloatImage::itkImageType>::New();
  filter->SetInput(mImageStack->image(ui.denoisingInputComboBox->currentIndex())->itkImage());
  filter->SetConductanceParameter(ui.conductanceSpinBox->value());
  filter->SetTimeStep(0.062);
  filter->SetNumberOfIterations(ui.iterationsSpinBox->value());
  
  // MULTITHREADING: Pass the filter object and a description to the
  // QThread object.  The description will be used later to create GUI
  // menu entries for the output of the filtering.
  mITKFilteringThread->setFilter(filter);
  mITKFilteringThread->setDescription(mImageStack->name(ui.denoisingInputComboBox->currentIndex()) 
                                      + QString(" (curvature anisotropic)"));
  mITKFilteringThread->start();
}

void wseGUI::runGradientFiltering()
{
  this->output(QString("Running the gradient magnitude filter."));

  itk::GradientMagnitudeImageFilter<FloatImage::itkImageType, FloatImage::itkImageType>::Pointer filter = 
    itk::GradientMagnitudeImageFilter<FloatImage::itkImageType, FloatImage::itkImageType>::New();
  filter->SetInput(mImageStack->image(ui.gradientInputComboBox->currentIndex())->itkImage());
  filter->SetUseImageSpacingOff();
 
  // MULTITHREADING: Pass the filter object and a description to the
  // QThread object.  The description will be used later to create GUI
  // menu entries for the output of the filtering.
  mITKFilteringThread->setFilter(filter);
  mITKFilteringThread->setDescription(mImageStack->name(ui.gradientInputComboBox->currentIndex()) 
                                      + QString(" (gradient)"));
  mITKFilteringThread->start();
}

void wseGUI::runWatershedSegmentation()
{
  this->output("Running the watershed segmentation filter");
  
  // Warn the user if we are about to delete any existing segmentation
  // data.  (Data is not actually deleted until successful completion
  // of the QThread run
  if (mSegmentation != NULL)
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText("This operation will delete your current watershed segmentation. You will lose any unsaved data.");
      msgBox.setInformativeText("Do you want to proceed?");
      msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      int ret = msgBox.exec();
      
      if (ret == QMessageBox::Cancel)
        {
          this->output("Cancelled the watershed segmentation filter.");
          return;
        }
    } // end if mSegmentation != NULL
  
  itk::WatershedImageFilter<FloatImage::itkImageType>::Pointer filter = 
    itk::WatershedImageFilter<FloatImage::itkImageType>::New();
  filter->SetInput(mImageStack->image(ui.watershedInputComboBox->currentIndex())->itkImage());
  filter->SetThreshold(ui.histogramSlider_1->getLowerThreshold());
  filter->SetLevel(ui.histogramSlider_1->getUpperThreshold());
  
  // MULTITHREADING: Pass the filter object and a description to the
  // QThread object.  The description will be used later to create GUI
  // menu entries for the output of the filtering.
  mITKSegmentationThread->setFilter(filter);
  mITKSegmentationThread->setDescription(mImageStack->name(ui.watershedInputComboBox->currentIndex())
                                         + QString(" (watershed transform)"));
  mITKSegmentationThread->start();
}

void wseGUI::mITKSegmentationThread_started()
{ 
  // Hook up to itk progress event
  itk::wseITKCallback<wseGUI>::Pointer mycmd = itk::wseITKCallback<wseGUI>::New();
  mycmd->setGui(this);    
  mITKSegmentationThread->filter()->AddObserver(itk::ProgressEvent(), mycmd);
  
  // Show progress bar
  ui.progressBar->setValue(0);
  ui.progressBar->show();
}
  
void wseGUI::mITKSegmentationThread_finished()
{ 
  // First rerun the filter to a zero level so that we get the base
  // transform on the WatershedImageFilter output.  This is a bit of a
  // hack, but won't introduce much overhead, since it only causes a
  // relabeling of the image (the transform and merge tree will not be
  // recomputed because level 0.0 is lower than the original level).
  // Note that the non-hack alternative to this step would be to write
  // another version of WatershedImageFilter that doesn't relabel its
  // output image.
  dynamic_cast<itk::WatershedImageFilter<FloatImage::itkImageType> *>
    (mITKSegmentationThread->filter().GetPointer())->SetLevel(0.0);
  mITKSegmentationThread->filter()->UpdateLargestPossibleRegion();
  
  // Now wrap up...
  ui.progressBar->setValue(100);
  ui.progressBar->hide();
  if (mITKSegmentationThread->errorFlag() == true)
    {
      QMessageBox::warning(this, tr("Segmentation aborted"), mITKSegmentationThread->errorString());
      this->output("Filter operation aborted by user");
    }  
  this->output("Segmentation operation finished");
  
  // First clean up old segmentation
  if (mSegmentation != NULL) { delete mSegmentation; }
  
  // Create the segmentation object.
  
  ULongImage *img = new ULongImage(mITKSegmentationThread->filter()->GetOutput());
  img->name(mITKSegmentationThread->description());
  
  //  mSegmentation = new Segmentation(mITKSegmentationThread->filter()->GetOutput(),
  mSegmentation = new Segmentation(img, dynamic_cast<itk::WatershedImageFilter<FloatImage::itkImageType> *>
                                   (mITKSegmentationThread->filter().GetPointer())->GetSegmentTree());
  
  this->setDualView();
  ui.floodLevelA->setEnabled(true);
  ui.floodLevelB->setEnabled(true);
  
  this->updateImageDisplay();
}
  
} //end namespace wse
