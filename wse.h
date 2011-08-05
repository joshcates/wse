#ifndef WSE_H
#define WSE_H

// TODO clean up VTK objects in destructor -jc

#include <QtGui/QMainWindow>
#include <QtGui/QProgressBar>
#include <QSettings>
#include <QDebug>
#include "ui_wse.h"
#include <vector>

// for the image viewer
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

// For the isosurface rendering
#include "vtkContourFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkImageToStructuredPoints.h"
#include "vtkProperty.h"
#include "vtkImageImport.h"
#include "vtkImageStencilData.h"
#include "vtkImageToImageStencil.h"
#include "vtkImageMapper.h"
#include "vtkActor2D.h"
#include "vtkLookupTable.h"
#include "vtkPolyDataNormals.h"
#include "vtkMarchingCubes.h"
#include "vtkPointPicker.h"

#include "exception.h"
#include "colorScheme.h"
#include "colorMaps.h"
#include "histogram.h"
#include "imageStack.h"
#include "SliceViewer.h"

#include "qwt_color_map.h"
#include "qwt_scale_widget.h"


// ITK includes
#include "itkImage.h"

//#include "IsoRenderer.h"

//class IsoRenderer;

#define ISO_NOT_SHOWN 0
#define ISO_SURFACE 1
#define ISO_MESH 2
#define ISO_POINT 3


namespace wse {

class InteractorCallback;

/** */
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

/** */
class wseGUI : public QMainWindow
{
  Q_OBJECT
  
public:
  typedef Image::itkFloatImage itkFloatImage;
  wseGUI(QWidget *parent = 0, Qt::WFlags flags = 0);
  ~wseGUI();
  
  void init();
  void closeEvent(QCloseEvent *event);
  
  void updateImageDisplay();
  bool addImageFromFile(QString fname);
  bool addImageFromData(Image *img);

  static QSettings *g_settings;

public slots:
  void visualizePage();

  void importDelete();
  void imageDropped(QString);
  void exportListItemChanged(QListWidgetItem *item);
  void exportImage();
  void saveLayout();
  void loadLayout();
  void visClassCheckAll();
  void visClassUncheckAll();
  void visImageCheckAll();
  void visImageUncheckAll();
  void exportCheckAll();
  void exportUncheckAll();
  void updateProgress(int p);
  void visSelectionChanged(QListWidgetItem *item);
  void visMethodChanged(int m);
  void colorSchemeSelectorChanged(int m);
  void viewerChangeSlice();
  void updateHistogram();
  void updateImageListIcons();
  void numBinsSpinnerChanged(int n);
  void thresholdChanged(double lowerRatio, double upperRatio);

  void thresholdTimerEvent();
  void toggleFullScreen();
  void setNormalView();
  void setDualView();
  void setSliceView();
  void setIsoSurfaceView();
  void toggleThresholdDisplay();
  void togglePercentageShown();
  void pointPick();

  void changeSlice(bool direction);

  /** Updates all registered combo boxes with all names of loaded data */
  void populateImageDataComboBoxes();

  /** Sets the image for isosurfacing and updates the rendering pipeline */
  void setIsosurface();
  
  /** Sets the image to use for masking out the data image (i.e. the segmentation) */
  void setImageMask();
  
  //  void snapToFibrosisMarker(int num);

  void colorMapChanged(int m);

  void reportThreshold();

  //float cmdLineComputeIntensity(std::string data, std::string mask, float percent);
  //float cmdLineComputePercentage(std::string data, std::string mask, float intensity);

private:
  void setupUI();
  void setupWatershedWindowUI();
  void readSettings();
  void writeSettings();
  void redrawIsoSurface();
  void updateHistogramWidget();
  void updateHistogramBars();
  void updatePercentageDisplay();

  void setThresholdToIntensity(double value);
  void setThresholdToIntensity(double lower, double upper);

  void showStatusMessage(const QString &text, int timeout = 0);

  void setMouseHandling();

  void updateColorMap();

  // float computePercentForIntensity(float lower, float upper);
  // void computePercentForIntensity(float lower, float upper, float &percentage, unsigned long &pixelCount);

  /** The main user interface class */
  Ui::WSEClass ui;

  /** Actions triggered from the main interface */
  QAction *mImportAction;
  //  QAction *mExportAction;
  QAction *mImportImageAction;
  //  QAction *mExportColormapAction;
  QAction *mFullScreenAction;
  QAction *mNormalView;
  QAction *mDualView;
  QAction *mSliceView;
  QAction *mIsoSurfaceView;
  //  QAction *mToggleThresholdAction;
  // QAction *mTogglePercentageShownAction;
  QAction *mViewControlWindowAction;
  QAction *mViewDataWindowAction;
  QAction *mViewWatershedWindowAction;
  QAction *mEditAddAction;
  QAction *mEditSubtractAction;
  QAction *mEditMergeAction;
  QAction *mEditUndoMergeAction;
  
  /** The stack of image volumes in memory */
  imageStack *mImageStack;

  /** */
  int mMinHistogramBins;

  /** */
  int mMaxHistogramBins;

  /** The class that computes and stores a histogram from an ITK image */
  Histogram<Image::itkFloatImage> *mHistogram;

  bool mSmoothStepThreshold;
  QwtScaleWidget mScaleWidget;
  int mCurrentColorMap;

  QProgressBar *mProgressBar;

  SliceViewer *mSliceViewer;
  vtkImageData *mNullVTKImageData;

  vtkActor *mCrosshairActor;
  
  //  IsoRenderer *mIsoRenderer;

  std::vector<QComboBox *> mRegisteredComboBoxes;

  Image mThresholdImage;
  int mIsosurfaceImage;
  int mImageData;
  int mImageMask;  
  colorSchemes mColorSchemes;
  colorMaps mColorMaps;


  QString mPercentageString;

  InteractorCallback *mVTKCallback;
  vtkPointPicker *mPointPicker;

  bool mPercentageShown;
  bool mFullScreen;


  std::vector<int> mPoints;
  std::vector<float> mMarkers;
  std::vector<double> mPercentages;

  float mThresholdLower;
  float mThresholdUpper;

  QTimer mThresholdTimer;
  
  QwtLinearColorMap mColorMap;

private slots:
  /** Slots for the watersheds denoising interface. */
  void on_gaussianRadioButton_toggled(bool);
  void on_anisotropicRadioButton_toggled(bool);
  void on_curvatureRadioButton_toggled(bool);
  void on_executeDenoisingButton_accepted();
  void on_executeDenoisingButton_rejected(){};

  /** Slots for the Data Manager window */
  void on_imageListWidget_itemSelectionChanged();
  void on_setImageDataButton_released();
  void on_addButton_released();


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

  //  void on_lowerThresholdSpinBox_valueChanged(double);
  // void on_upperThresholdSpinBox_valueChanged(double);

protected:
  // override the wheelEvent to move slices with the mousewheel
  void wheelEvent(QWheelEvent *event);

};

} // end namespace wse

#endif // WSE_H
