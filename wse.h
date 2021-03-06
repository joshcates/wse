#ifndef WSE_H
#define WSE_H

// Standard includes
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

// Qt includes
#include <QtGui/QMainWindow>
#include <QtGui/QProgressBar>
#include <QSettings>
#include <QDebug>
#include "ui_wse.h"

// VTK includes
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkImageData.h"
#include "vtkImageImport.h"
#include "vtkImageMapper.h"
#include "vtkLookupTable.h"
#include "vtkMarchingCubes.h"
#include "vtkPointPicker.h"
#include "vtkImageBlend.h"
#include "vtkMarchingCubes.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkSmartPointer.h"
#include "vtkProp3DCollection.h"


// ITK includes
#include "itkImage.h"
#include "itkCommand.h"
#include "QThreadITKFilter.hxx"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkWatershedImageFilter.h"

// WSE includes
#include "wseImage.hxx"
#include "wseImageStack.hxx"
#include "wseException.h"
#include "colorScheme.h"
#include "colorMaps.h"
#include "wseHistogram.hxx"
#include "wseSliceViewer.h"
#include "wseSegmentationViewer.h"
//#include "IsoRenderer.h"
#include "wseUtils.h"
#include "wseSegmentation.h"

// QWT includes
//#include "qwt_color_map.h"
//#include "qwt_scale_widget.h"
//#include "qwt_scale_engine.h"

//#include "IsoRenderer.h"

//class IsoRenderer;

#define ISO_NOT_SHOWN 0
#define ISO_SURFACE 1
#define ISO_MESH 2
#define ISO_POINT 3

namespace wse {

class InteractorCallback;

/** This is the wseApplication class to override notify() and
    loadStyleSheet(). */
class wseApplication : public QApplication
{
public:
  wseApplication(int &argc, char **argv) : QApplication(argc, argv) {}
  ~wseApplication() {}

  void loadStyleSheet(const char *fn);

  bool notify(QObject *receiver, QEvent *e)
  {
    try
    {
      return QApplication::notify(receiver, e);
    }
    catch(Exception &cve)
    {
      qDebug() << "WSE caught an exception from " << receiver->objectName() 
               << " from event type " << e->type() << ".\n" << cve.info.c_str();
      qFatal("Exiting due to unrecoverable error.");
      return false;
    }
    catch(...)
    {
      qDebug() << "WSE caught an exception from " << receiver->objectName() 
               << " from event type " << e->type();
      qFatal("Exiting due to unrecoverable error.");
      return false;
    }
  }
};

/** This class implements the Watershed Editor GUI layout, logic, and image processing. */
class wseGUI : public QMainWindow
{
  Q_OBJECT
  
public:
  /** The type of itk::Image that we are working with. */
  typedef FloatImage::itkImageType itkImageType;

  wseGUI(QWidget *parent = 0, Qt::WFlags flags = 0);
  ~wseGUI();
  
  /**  */
  void closeEvent(QCloseEvent *event);
  
  /** Updates and re-renders all of the viewer windows. */
  void updateImageDisplay();

  /** Add an image to the image stack from a file.  The argument is
      the filename.  Returns true on success and false otherwise. */
  bool addImageFromFile(QString fname);

  /** Add an image to the image stack from a FloatImage pointer.
      Responsibility for freeing the image data is passed to this
      class.*/
  bool addImageFromData(FloatImage *img);

  /** User and window layout settings for this GUI. These are saved on
      exit and reloaded when the program is executed.*/
  static QSettings *g_settings;

  /** Returns a pointer to the progress bar.  This method is require
      for compatibility with wseITKCallback */
  QProgressBar *getProgressBar() { return ui.progressBar; }

  /** This method can be called when an application using this GUI is
      first started to give the user some guidance.  */
  void showStartMenu()  { this->on_addButton_released(); }

public slots:
  void importDelete();
  void imageDropped(QString);
  void visClassCheckAll();
  void visClassUncheckAll();
  void visImageCheckAll();
  void visImageUncheckAll();
  void exportCheckAll();
  void exportUncheckAll();
  void updateProgress(int p);
  void colorSchemeSelectorChanged(int m);
  void viewerChangeSlice();
  void updateHistogram();
  void updateImageListIcons();
  void numBinsSpinnerChanged(int n);
  void thresholdChanged(double lowerRatio, double upperRatio);

  /** Arrange the GUI windows in several preset ways. */
  void toggleFullScreen();
  void setNormalView();
  void setDualView();
  void setSliceView();
  void setIsoSurfaceView();

  /** Picking callback for the 3D render window. */
  void pointPick3D();

  /** Picking callback for the 2D segment render window. */
  void cellPickSegment(float x, float y, bool repick, vtkRenderer *ren);

  void changeSlice(bool direction);

  /** Syncronizes all registered combo boxes
      (mRegisteredImageComboBoxes) with the names of image data in the
      ui.imageListWidget.*/
  void syncRegisteredImageComboBoxes();

  /** Syncronizes all registered combo boxes
      (mRegisteredSegmentationComboBoxes) with the names of data in
      the ui.segmentationListWidget. */
  void syncRegisteredSegmentationComboBoxes();

  /** Sets the image for isosurfacing and updates the rendering pipeline */
  void setIsosurface();
  
  /** Sets the image to use for masking out the data image (i.e. the segmentation) */
  void setImageMask();

  /** */  
  void colorMapChanged(int m);

protected:
  /** Overrides the wheelEvent to move slices with the mousewheel */
  void wheelEvent(QWheelEvent *event);

private:
  /** The main user interface class */
  Ui::WSEClass ui;

  /** The URL for welcome text */
  QUrl mWelcomeUrl;

  /** The URL for help text */
  QUrl mHelpUrl;

  /** Actions triggered from the main interface */
  QAction *mExportImageAction;
  QAction *mImportImageAction;
  QAction *mImportWSSegmentationAction;
  //  QAction *mExportColormapAction;
  QAction *mFullScreenAction;
  QAction *mNormalView;
  QAction *mDualView;
  QAction *mSliceView;
  QAction *mIsoSurfaceView;
  QAction *mViewControlWindowAction;
  QAction *mViewDataWindowAction;
  QAction *mViewWatershedWindowAction;
  QAction *mViewConsoleWindowAction;
  QAction *mEditAddAction;
  QAction *mEditSubtractAction;
  QAction *mEditMergeAction;
  QAction *mEditUndoMergeAction;
  QAction *mHelpAction;
  
  /** The stack of image volumes in memory */
  FloatImageStack *mImageStack;

  /** The watershed segmentation.  This object is NULL if no
      segmentation exists. This version of wse only supports one
      segmentation at a time.*/
  Segmentation *mSegmentation;

  /** The class that computes and stores a histogram from an ITK image */
  Histogram<itkImageType> *mHistogram;

  /** */
  int mMinHistogramBins;

  /** */
  int mMaxHistogramBins;

  /** TODO: Document */
  //  QwtScaleWidget mScaleWidget;

  /** TODO: Document */
  int mCurrentColorMap;

  /** The main progress bar for the GUI.  */
  QProgressBar *mProgressBar;

  /** The slice-by-slice image viewer for the floating point data
      volumes. */
  SliceViewer *mSliceViewer;

  /** The slice-by-slice image viewer for the watershed segmentation
      output */
  SegmentationViewer *mSegmentSliceViewer;

  /** TODO: Document */
  vtkImageData *mNullVTKImageData;

  /** TODO: Document */
  vtkActor *mCrosshairActor;
  
  //  IsoRenderer *mIsoRenderer;

  /** This list of combo boxes is kept in sync with the master list of
      images from ui.imageListWidget. */
  std::vector<QComboBox *> mRegisteredImageComboBoxes;

  /** This list of combo boxes is kept in sync with the master list of
      images from ui.imageListWidget. */
  std::vector<QComboBox *> mRegisteredSegmentationComboBoxes;

  /** The integer number of the image in the mImageStack that is
      currently displayed.  If this number is -1, no image is being
      displayed. */
  int mImageData;

  /** TODO: Document */
  int mImageMask;  

  /** TODO: Document */
  int mIsosurfaceImage;

  /** TODO: Document */
  colorSchemes mColorSchemes;

  /** TODO: Document */
  colorMaps mColorMaps;

  /** TODO: Document */
  InteractorCallback *mVTKCallback;

  /** Picker object for the 3D Render window */
  vtkPointPicker *mPointPicker3D;

  /** Picker object for the 2D Segmentation window */
  vtkPointPicker *mCellPickerSegment;

  /** TODO: Document */
  bool mFullScreen;

  /** TODO: Document */
  std::vector<int> mPoints;

  /** TODO: Document */
  float mThresholdLower;

  /** TODO: Document */
  float mThresholdUpper;

  /** TODO: Document */
  //  QwtLinearColorMap mColorMap;

  /** The threading object for image-to-image filters. TODO: Redefine
      the ThreadITKFilter to handle both of these image filters in one
      object. */
  itk::QThreadITKFilter<itk::ImageToImageFilter<FloatImage::itkImageType,FloatImage::itkImageType> > 
    *mITKFilteringThread;

  /** The threading object for image-to-segmentation filters. TODO:
      Redefine the ThreadITKFilter to handle both of these image
      filters in one object. */
  itk::QThreadITKFilter<itk::ImageToImageFilter<FloatImage::itkImageType,ULongImage::itkImageType> > 
    *mITKSegmentationThread;

  /** Perform Gaussian filtering on a selected image. */
  void runGaussianFiltering();

  /** Perform Anisotropic Diffusion filtering on a selected image. */
  void runAnisotropicFiltering();

  /** Perform Curvature Anisotropic Diffusion filtering on a selected image. */
  void runCurvatureFiltering();

  /** Run the gradient magnitude image filter. */
  void runGradientFiltering();

  /** Run the watershed segmentation filter */
  void runWatershedSegmentation();

  /** Requests termination of filtering operations. */
  void requestAbortITKFilter()
  {
    if ( mITKFilteringThread->isFiltering() ) 
      {  mITKFilteringThread->filter()->SetAbortGenerateData(true); }
    if ( mITKSegmentationThread->isFiltering() ) 
      {  mITKSegmentationThread->filter()->SetAbortGenerateData(true); }
  }

  /** Write a string to the console output window. */
  void output(const char *s)  
  {  
    ui.outputConsole->append(QTime::currentTime().toString() + QString("> ") + QString(s)); 
    ui.outputConsole->update();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  }
  void output(const QString &s)  
  { 
    ui.outputConsole->append(QTime::currentTime().toString() + QString("> ") + s);  
    ui.outputConsole->update();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  }
  void output(const std::string &s)  
  { 
    ui.outputConsole->append(QTime::currentTime().toString() + QString("> ") + QString(s.c_str()) );  
    ui.outputConsole->update();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  }

  /** Perform the initial setup of the user interface */
  void setupUI();
  void setupUIMenu();
  void setupUIWatershedWindow();
  void setupUIDataManager();

  /** Read the user interface settings from disk. */
  void readSettings();

  /** Write the user interface settings to disk. */
  void writeSettings();

  /** TODO: Document. */
  void redrawIsoSurface();

  /** TODO: Document. */
  void updateHistogramWidget();

  /** TODO: Document. */
  void updateHistogramBars();

  /** Show a message in the status bar. */
  void showStatusMessage(const QString &text, int timeout = 0);

  /** TODO: Document. */
  void setMouseHandling();

  /** TODO: Document. */
  void updateColorMap();

private slots:
  void unimplemented()
  {  QMessageBox::warning(this, tr("Sorry"), QString("This function is not yet implemented."));  }

  /** Slots for histogram signals */
  void histogramThresholdChanged(double l, double h)
  {
    ui.lowerThresholdLabel->setText(QString("%1\%").arg(l*100.0,0,'g',4));
    ui.upperThresholdLabel->setText(QString("%1\%").arg(h*100.0,0,'g',4));
  }

  /** Slots for the watersheds denoising interface. */
  void on_gaussianRadioButton_toggled(bool);
  void on_anisotropicRadioButton_toggled(bool);
  void on_curvatureRadioButton_toggled(bool);
  void on_executeDenoisingButton_accepted();
  void on_executeDenoisingButton_rejected()
  { this->requestAbortITKFilter(); };
  void on_executeGradientButton_accepted();
  void on_executeGradientButton_rejected()
  { this->requestAbortITKFilter(); };
  void on_executeWatershedsButton_accepted();
  void on_executeWatershedsButton_rejected()
  { this->requestAbortITKFilter(); };
  
  /** Slots for the Data Manager window */
  void on_imageListWidget_itemSelectionChanged();
  void on_setImageDataButton_released();
  void on_addButton_released();
  void on_saveImageButton_released();

  void on_clipThresholdCheckBox_stateChanged(int );
  void on_thresholdOpacitySlider_valueChanged(int value);
  void on_showThresholdCheckBox_stateChanged(int );
  void on_imageMaskOpacitySlider_valueChanged(int value);
  void on_showMaskCheckBox_stateChanged(int );
  void on_showNormalsCheckBox_stateChanged(int );
  void on_antiAliasIterationsSlider_valueChanged(int);
  void on_gaussianVarianceSlider_valueChanged(int);
  void on_clipIsoSurfaceCheckBox_stateChanged(int value);
  void on_showMaskOnIsoSurface_stateChanged(int value);
  void on_clipIsoSurfaceComboBox_currentIndexChanged(int index);
  void on_endoSurfaceComboBox_currentIndexChanged(int index);
  void on_wallSurfaceComboBox_currentIndexChanged(int index);
  void on_scalarSurfaceComboBox_currentIndexChanged(int index);

  void on_imageInterpolationComboBox_currentIndexChanged(int index);
  void on_maskInterpolationComboBox_currentIndexChanged(int index);

  void on_histogramBarsComboBox_currentIndexChanged(int index);
  void on_histogramColorScaleCheckBox_stateChanged(int);
  void on_snapColorsToBarsCheckBox_stateChanged(int);
  void on_smoothGroupBox_toggled(bool expanded);
  void on_advancedOptionsGroupBox_toggled(bool expanded);


  /** */
  void on_floodLevelA_valueChanged(int) 
  { this->floodLevelChanged(); }
  void on_floodLevelB_valueChanged(int)
  { this->floodLevelChanged(); }
  //  void on_floodLevelSpinBox_editingFinished(double)
  // void on_floodLevelSpinBox_valueChanged(double lvl)
  // {
  //  ui.floodLevelA->setValue(floor(lvl));
  //  ui.floodLevelA->setValue(lvl - floor(lvl));
  // }
  void floodLevelChanged();

  void displayHelp();

  // Won't connect automatically
  void mITKFilteringThread_finished();
  void mITKFilteringThread_started();
  void mITKSegmentationThread_finished();
  void mITKSegmentationThread_started();

}; // end class wseGUI

} // end namespace wse

#endif // WSE_H
