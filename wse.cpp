#include "wse.h"
#include "image.h"
#include "imageStack.h"
#include <QDebug>
#include "vtkImageData.h"
#include <iostream>
#include <vector>
#include <iostream>
#include <sstream>

#include "itkScalarImageToListAdaptor.h"
#include "itkListSampleToHistogramGenerator.h"
#include "itkScalarImageToHistogramGenerator.h"

#include "vtkProperty2D.h"
#include "vtkProperty.h"

#include "vtkImageAccumulate.h"
#include "vtkImageExtractComponents.h"
#include "vtkImageBlend.h"
#include "vtkMarchingCubes.h"

#include "itkImageDuplicator.h"
#include "vtkPolyDataNormals.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageActor.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkPointPicker.h"
#include "vtkCellArray.h"

#include <limits>

#include "utils.h"

#include "itkImage.h"

#include "vtkSmartPointer.h"
#include "vtkHedgeHog.h"
#include "vtkPlane.h"

// Mesh subdivision
//#include "vtkLinearSubdivisionFilter.h"
//#include "vtkButterflySubdivisionFilter.h"
//#include "vtkLoopSubdivisionFilter.h"

//#include "quantification.h"

#include "qwt_color_map.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_engine.h"

//#include "IsoRenderer.h"

namespace wse {

QSettings *wseGUI::g_settings;

/** */
class InteractorCallback : public vtkCommand 
{
private:

  wseGUI *wse_;

public:
  InteractorCallback(wseGUI *wse) 
  {
    wse_ = wse;
  }

  virtual void Execute(vtkObject *, unsigned long event, void *) 
  {
    if (event == vtkCommand::MouseWheelBackwardEvent) 
      {
	wse_->changeSlice(false);
      } 
    else if (event == vtkCommand::MouseWheelForwardEvent) 
      {
	wse_->changeSlice(true);
      }
  }
};

/** */
class PickerCallback : public vtkCommand 
{
private:

  wseGUI *wse_;

public:
  PickerCallback(wseGUI *wse) 
  {
    wse_ = wse;
  }
  
  virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData) 
  {
    wse_->pointPick();
  }
};
  
wseGUI::wseGUI(QWidget *parent, Qt::WFlags flags) :
  QMainWindow(parent, flags),
  mImageStack(NULL),
  mMinHistogramBins(10),
  mMaxHistogramBins(1000),
  mHistogram(NULL),
  mExportDirty(true),
  mSmoothStepThreshold(false),
  mScaleWidget(QwtScaleDraw::BottomScale, this), mCurrentColorMap(0)
{
  mIsosurfaceImage = NULL;
  mSliceViewer = SliceViewer::New();
  mNullVTKImageData = vtkImageData::New();
  
  mImageData = -1;
  mImageMask = -1;
  mIsosurfaceImage = -1;
  //  mScalarMethod = 0;
  mFullScreen = false;
  mPercentageShown = false;

  this->init();
}

wseGUI::~wseGUI()
{
  if (mImageStack) { delete mImageStack; }
  //mVTKImageViewer->Delete();
  mSliceViewer->Delete();
  mNullVTKImageData->Delete();
}

void wseGUI::init()
{
#ifdef WIN32
  RedirectIOToConsole();
#endif

  ui.setupUi(this);
  readSettings();
  mImageStack = new imageStack();
  
  setupUI();

  this->setSliceView();
  
//#define LOAD_SAMPLE_DATA
#ifdef LOAD_SAMPLE_DATA
  //addImageFromFile(QString("/Users/amorris/carma/data/AFIB_DATA/demri.nrrd"));
  //addImageFromFile(QString("/Users/amorris/carma/data/AFIB_DATA/LA_endo-seg.nrrd"));
  //addImageFromFile(QString("/Users/amorris/carma/data/AFIB_DATA/LA_Output.nrrd"));

  //addImageFromFile(QString("/Users/amorris/carma/data/3mo/export/DEMRI.nrrd"));
  //addImageFromFile(QString("/Users/amorris/carma/data/3mo/export/donut.nrrd"));
  ////addImageFromFile(QString("/Users/amorris/carma/data/3mo/export/donut-cleaned-up.nrrd"));
  //addImageFromFile(QString("/Users/amorris/carma/data/3mo/export/endo.nrrd"));

  //addImageFromFile(QString("/Users/amorris/carma/data/Weird Example/LGE.nrrd"));
  //addImageFromFile(QString("/Users/amorris/carma/data/Weird Example/wall-seg.nrrd"));
  //addImageFromFile(QString("/Users/amorris/carma/data/Weird Example/endo-seg.nrrd"));

  //addImageFromFile(QString("/Users/amorris/carma/data/scar_crash/LGE.nrrd"));
  //addImageFromFile(QString("/Users/amorris/carma/data/scar_crash/Donut.nrrd"));
  //addImageFromFile(QString("/Users/amorris/carma/data/scar_crash/Endo.nrrd"));

  addImageFromFile(QString("/Users/amorris/carma/data/Wse Error 122010/demri.nrrd"));
  addImageFromFile(QString("/Users/amorris/carma/data/Wse Error 122010/wall.nrrd"));
  addImageFromFile(QString("/Users/amorris/carma/data/Wse Error 122010/endo.nrrd"));

  mImageData = 0;
  mImageMask = 1;
  mIsosurfaceImage = 2;

  ui.setImageMaskButton->setEnabled(true);
  updateImageListIcons();
  updateImageDisplay();
#endif

  // Register image selection combo box widgets to stay in sync with file names
  mRegisteredComboBoxes.push_back(ui.denoisingInputComboBox);
  mRegisteredComboBoxes.push_back(ui.watershedInputComboBox);
  mRegisteredComboBoxes.push_back(ui.gradientInputComboBox);
}

void wseApplication::loadStyleSheet(const char *fn)
{
  QString sheetName(fn);
  QFile file(fn);
  file.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(file.readAll());

  qApp->setStyleSheet(styleSheet);
}


void wseGUI::setupUI() 
{
  // ui.dataDockWidget->setWindowFlags(Qt::Drawer);

  // set up the isosurface renderer first
  // mIsoRenderer = new IsoRenderer();
  // ui.vtkRenderWidget->GetRenderWindow()->AddRenderer(mIsoRenderer->getSurfaceRenderer());
  // ui.vtkRenderWidget->show();

  PickerCallback *mPickerCallback = new PickerCallback(this);
  mPointPicker = vtkPointPicker::New();
  mPointPicker->AddObserver(vtkCommand::EndPickEvent, mPickerCallback);
  ui.vtkRenderWidget->GetInteractor()->SetPicker(mPointPicker);

  // Toolbar
  mImportAction = new QAction(QIcon(":/WSE/Resources/import.png"), tr("&Import"), this);
  mImportAction->setToolTip(tr("Import"));
  mImportAction->setStatusTip(tr("Import registered images"));
  mImportAction->setCheckable(true);
  
  mExportAction = new QAction(QIcon(":/WSE/Resources/export.png"), tr("&Export"), this);
  mExportAction->setToolTip(tr("Export"));
  mExportAction->setStatusTip(tr("Export model"));
  mExportAction->setCheckable(true);
  connect(mExportAction, SIGNAL(triggered()), this, SLOT(exportPage()));

  QActionGroup *toolBarGroup = new QActionGroup(this);
  toolBarGroup->addAction(mImportAction);
  toolBarGroup->addAction(mExportAction);
  mImportAction->setChecked(true);
  mExportAction->setEnabled(false);

  // ui.mainToolBar->addAction(mImportAction);
  // ui.mainToolBar->addAction(mExportAction);
  // ui.mainToolBar->setIconSize(QSize(30,30));

  // Menubar
  mImportImageAction = new QAction(tr("Import Image"), this);
  mImportImageAction->setShortcut(tr("Import images from file"));
  mExportColormapAction = new QAction(tr("Export Colormap"), this);
  mExportColormapAction->setShortcut(tr("Export selected colormaps"));
  mExportColormapAction->setEnabled(false);
  QAction *exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+Q"));
  exitAction->setStatusTip(tr("Exit the application"));
  connect(mImportImageAction, SIGNAL(triggered()), this, SLOT(importAdd()));
  connect(mExportColormapAction, SIGNAL(triggered()), this, SLOT(exportImage()));
  connect(exitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

  mFullScreenAction = new QAction(tr("&Full Screen"), this);
  mFullScreenAction->setShortcut(tr("Ctrl+F"));
  mFullScreenAction->setCheckable(true);

  connect(mFullScreenAction, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

  mNormalView = new QAction(tr("Go to &Normal View"), this);
  mNormalView->setShortcut((tr("Ctrl+1")));
  connect(mNormalView, SIGNAL(triggered()), this, SLOT(setNormalView()));

  mDualView = new QAction(tr("Go to &Dual View"), this);
  mDualView->setShortcut((tr("Ctrl+2")));
  connect(mDualView, SIGNAL(triggered()), this, SLOT(setDualView()));

  mSliceView = new QAction(tr("Go to &Slice View"), this);
  mSliceView->setShortcut((tr("Ctrl+3")));
  connect(mSliceView, SIGNAL(triggered()), this, SLOT(setSliceView()));

  mIsoSurfaceView = new QAction(tr("Go to &IsoSurface View"), this);
  mIsoSurfaceView->setShortcut((tr("Ctrl+4")));
  connect(mIsoSurfaceView, SIGNAL(triggered()), this, SLOT(setIsoSurfaceView()));

  mToggleThresholdAction = new QAction(tr("Toggle Threshold Display"), this);
  mToggleThresholdAction->setShortcut(Qt::Key_Space);
  mToggleThresholdAction->setCheckable(true);
  connect(mToggleThresholdAction, SIGNAL(triggered()), this, SLOT(toggleThresholdDisplay()));

  mTogglePercentageShownAction = new QAction(tr("Toggle Percentage Display"), this);
  mTogglePercentageShownAction->setShortcut(tr("Ctrl+S"));
  mTogglePercentageShownAction->setCheckable(true);
  connect(mTogglePercentageShownAction, SIGNAL(triggered()), this, SLOT(togglePercentageShown()));

  QMenu *fileMenu = ui.menuBar->addMenu(tr("&File"));
  fileMenu->addAction(mImportImageAction);
  fileMenu->addAction(mExportColormapAction);
  fileMenu->addAction(exitAction);
  
  // QMenu *toolsMenu = ui.menuBar->addMenu(tr("&Tools"));
  // toolsMenu->addAction(mImportAction);
  // toolsMenu->addAction(mExportAction);

  mViewDataWindowAction = new QAction(tr("View Data Window"), this);
  connect(mViewDataWindowAction, SIGNAL(triggered()), ui.dataDockWidget, SLOT(show()));

  mViewControlWindowAction = new QAction(tr("View Control Window"), this);
  connect(mViewControlWindowAction, SIGNAL(triggered()), ui.controlsDockWidget, SLOT(show()));

  mViewWatershedWindowAction = new QAction(tr("View Watershed Window"), this);
  connect(mViewWatershedWindowAction, SIGNAL(triggered()), ui.watershedDockWidget, SLOT(show()));

  QMenu *viewMenu = ui.menuBar->addMenu(tr("&View"));
  viewMenu->setTearOffEnabled(true);
  viewMenu->setSeparatorsCollapsible(true);
  viewMenu->addAction(mFullScreenAction);
  viewMenu->addAction(mNormalView);
  viewMenu->addAction(mDualView);
  viewMenu->addAction(mSliceView);
  viewMenu->addAction(mIsoSurfaceView);
  viewMenu->addAction(mToggleThresholdAction);
  viewMenu->addAction(mTogglePercentageShownAction);
  viewMenu->addSeparator();
  viewMenu->addAction(mViewDataWindowAction);
  viewMenu->addAction(mViewWatershedWindowAction);
  viewMenu->addAction(mViewControlWindowAction);

  // Construct the edit menu
  mEditAddAction       = new QAction(tr("Add"), this);
  mEditSubtractAction  = new QAction(tr("Subtract"),this);
  mEditMergeAction     = new QAction(tr("Merge"),this);
  mEditUndoMergeAction = new QAction(tr("Undo Merge"),this);

  QMenu *editorMenu = ui.menuBar->addMenu(tr("&Editor"));
  editorMenu->addAction(mEditAddAction);
  editorMenu->addAction(mEditSubtractAction);
  editorMenu->addAction(mEditMergeAction);
  editorMenu->addAction(mEditUndoMergeAction);


  //  QSignalMapper *mapper = new QSignalMapper(this);

  // for (int i=0; i<5; ++i) {
  //   QString description = QString("Snap to score #") + QString::number(i+1);
  //   QString shortcut = QString("f") + QString::number(i+1);
  //   QAction *setToFibrosisScoreAction;
  //   setToFibrosisScoreAction = new QAction(tr(description), this);
  //   setToFibrosisScoreAction->setShortcut((tr(shortcut)));
  //   mapper->setMapping(setToFibrosisScoreAction,i);
  //   connect(setToFibrosisScoreAction, SIGNAL(triggered()), mapper, SLOT(map()));
  //   fibrosisMenu->addAction(setToFibrosisScoreAction);
  // }
  // connect (mapper, SIGNAL(mapped(int)), this, SLOT(snapToFibrosisMarker(int)));
 
  ui.menuBar->setEnabled(true);
  
  ui.deleteButton->setEnabled(false);
  //  ui.imageListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui.imageListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  ui.imageListWidget->setDragEnabled(true);
  ui.imageListWidget->setAcceptDrops(true);
  ui.imageListWidget->setDropIndicatorShown(true);
  ui.imageListWidget->setDragDropMode(QAbstractItemView::DropOnly);
  ui.imageListWidget->setStatusTip(tr("Select the '+' icon or drag and drop images"));

  //  connect(ui.imageListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(importSelectionChanged()));
  connect(ui.addButton, SIGNAL(released()), this, SLOT(importAdd()));
  connect(ui.deleteButton, SIGNAL(released()), this, SLOT(importDelete()));
  connect(ui.sliceSelector,SIGNAL(valueChanged(int)), this, SLOT(viewerChangeSlice()));
  ui.sliceSelector->setTracking(true);

  connect(ui.imageListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(setImageData()));
  
  ui.sliceSelector->setEnabled(false);
  
  connect(ui.imageListWidget, SIGNAL(imageDropped(QString)),
          this, SLOT(imageDropped(QString)));

  ui.vtkImageWidget->SetRenderWindow(mSliceViewer->GetRenderWindow());
  mSliceViewer->SetupInteractor(ui.vtkImageWidget->GetRenderWindow()->GetInteractor());


  mVTKCallback = new InteractorCallback(this);

  setMouseHandling();

  mSliceViewer->SetColorLevel(128);
  mSliceViewer->SetColorWindow(256);
  ui.vtkImageWidget->show();

  
  // Connect the processing buttons
  connect(ui.setIsosurfaceButton, SIGNAL(released()), this, SLOT(setIsosurface()));

  connect(ui.setImageMaskButton,  SIGNAL(released()), this, SLOT(setImageMask()));
  



  ui.setIsosurfaceButton->setEnabled(false);
  ui.setImageDataButton->setEnabled(false);
  ui.setImageMaskButton->setEnabled(false);
 

  // ... the color schemes
  for (unsigned int i = 0; i < mColorSchemes.size(); i++)
  {
    ui.colorSchemeSelector->addItem(QString(mColorSchemes[i].name.c_str()));
  }
  connect(ui.colorSchemeSelector, SIGNAL(currentIndexChanged(int)), this, 
    SLOT(colorSchemeSelectorChanged(int)));
          
          
  // Set up histogram bins, window, and level
  ui.numBinsSpinner->setMinimum(mMinHistogramBins);
  ui.numBinsSpinner->setMaximum(mMaxHistogramBins);
  ui.numBinsSpinner->setValue(mMaxHistogramBins/4);
  
  connect(ui.numBinsSpinner, SIGNAL(valueChanged(int)),this, SLOT(numBinsSpinnerChanged(int)));

  // Export page
  //   ui.exportClassListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  //   connect(ui.exportCheckButton, SIGNAL(released()),
  //           this, SLOT(exportCheckAll()));
  //   connect(ui.exportUncheckButton, SIGNAL(released()),
  //           this, SLOT(exportUncheckAll()));
  //   connect(ui.exportClassListWidget, SIGNAL(itemChanged(QListWidgetItem *)),
  //           this, SLOT(exportListItemChanged(QListWidgetItem *)));
  //   connect(ui.exportButton, SIGNAL(released()),
  //           this, SLOT(exportImage()));
  
  //  bool success = connect (ui.histogramSlider_1, SIGNAL(thresholdChanged(double,double)), this, 
  //  SLOT(thresholdChanged(double,double)));
  // assert(success);


  mThresholdTimer.setSingleShot(true);
  connect(&mThresholdTimer, SIGNAL(timeout()), this, SLOT(thresholdTimerEvent()));



  // the scalar schemes
  // std::vector<ScalarMethod> scalarMethods = ScalarMethod::getScalarMethods();
  // for (unsigned int i = 0; i < scalarMethods.size(); ++i) {
  //   ui.scalarMethodComboBox->addItem(QString(scalarMethods[i].name.c_str()));
  // }
  connect(ui.scalarMethodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(scalarMethodChanged(int)));

  for (unsigned int i = 0; i < mColorMaps.size(); ++i) {
    ui.colorMapComboBox->addItem(QString(mColorMaps[i].name.c_str()));
  }
  connect(ui.colorMapComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(colorMapChanged(int)));


  // Status Bar
  mProgressBar = new QProgressBar(this);
  mProgressBar->setRange(0,100);
  ui.statusBar->addPermanentWidget(mProgressBar);
  mProgressBar->hide();
  

  updateColorMap(); 

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(&mScaleWidget);
  vbox->setSpacing(0);
  vbox->setMargin(0);
  //  ui.colorScaleGroupBox->setLayout(vbox);



  mCrosshairActor = vtkActor::New();
  mSliceViewer->GetRenderer()->AddActor(mCrosshairActor);


  // set defaults for basic mode
  //ui.smoothGroupBox->setChecked(false);
  //ui.advancedOptionsGroupBox->setChecked(false);
  //ui.showMaskOnIsoSurface->setChecked(false);
  //ui.scalarMethodComboBox->setCurrentIndex(scalarMethods.size()-1);
  //ui.imageInterpolationComboBox->setCurrentIndex(1);
  //ui.maskInterpolationComboBox->setCurrentIndex(1);
  //ui.smoothThresholdCheckBox->setChecked(true);
  //ui.subdivideMeshCheckBox->setChecked(true);

  ui.smoothGroupBox->setChecked(false);
  ui.advancedOptionsGroupBox->setChecked(false);
  ui.showMaskOnIsoSurface->setChecked(false);
  ui.scalarMethodComboBox->setCurrentIndex(1);
  ui.imageInterpolationComboBox->setCurrentIndex(1);
  ui.maskInterpolationComboBox->setCurrentIndex(1);
  ui.smoothThresholdCheckBox->setChecked(false);
  ui.subdivideMeshCheckBox->setChecked(true);

  this->setupWatershedWindowUI();
}

void wseGUI::setupWatershedWindowUI()
{
  // Denoising UI setup
  ui.curvatureRadioButton->setChecked(true);
  ui.gaussianBlurringParamsBox->hide();

  //
}



void wseGUI::numBinsSpinnerChanged(int n) 
{
  this->updateHistogram();
}
  
  
void wseGUI::populateImageDataComboBoxes()
{
  // Loop through all registered combo boxes (those in a member variable list)
  for (unsigned int j = 0; j < mRegisteredComboBoxes.size(); j++)
    {
      // Clear all entries
      mRegisteredComboBoxes[j]->clear();
      // Populate with the names in the image list widget
      for (int i=0; i<ui.imageListWidget->count(); i++)
	{
	  mRegisteredComboBoxes[j]->addItem(ui.imageListWidget->item(i)->text());
	}
    }
}
  

/**
  Update the icons in the image list for the "image data" and "mask"
 */
void wseGUI::updateImageListIcons()
{
  QIcon dataIcon (QString::fromUtf8(":/WSE/Resources/DataVolume.png"));
  QIcon maskIcon (QString::fromUtf8(":/WSE/Resources/mask.png"));
  QIcon isoIcon (QString::fromUtf8(":/WSE/Resources/iso.png"));


  QIcon emptyIcon;

  for (int i=0; i<ui.imageListWidget->count(); i++) {
    if (i == mImageData) {
      // set the data volume icon on the selected image
      ui.imageListWidget->item(mImageData)->setIcon(dataIcon);
    } else if (i == mImageMask) {
      // set the mask icon on the selected image
      ui.imageListWidget->item(mImageMask)->setIcon(maskIcon);
    } else if (i == mIsosurfaceImage) {
      // set the mask icon on the selected image
      ui.imageListWidget->item(mIsosurfaceImage)->setIcon(isoIcon);
    } else {
      // set an empty icon on other images
      ui.imageListWidget->item(i)->setIcon(emptyIcon);
    }
  }
}


void wseGUI::on_setImageDataButton_released()
{
  mImageData = ui.imageListWidget->currentRow();
  //  ui.setImageDataButton->setEnabled(false);
  ui.setImageMaskButton->setEnabled(true);
  ui.sliceSelector->setEnabled(true);

  this->updateImageListIcons();
  this->updateImageDisplay();
  this->setSliceView();
}

void wseGUI::setImageMask()
{
  int selection = ui.imageListWidget->currentRow();

  if (mImageStack->image(selection)->isBinarySegmentation()) {
    mImageMask = selection;
    updateImageListIcons();
    updateImageDisplay();
  } else {
    //    int ret = 
    QMessageBox::critical(this, tr("WSE"),
      tr("Wall image must be a binary segmentation (values [0,1])"),QMessageBox::Ok);
  }
}

void wseGUI::setIsosurface()
{
  int selection = ui.imageListWidget->currentRow();

  if (mImageStack->image(selection)->isBinarySegmentation()) {
    mIsosurfaceImage = ui.imageListWidget->currentRow();//mImageStack->selectedImageVTK(true)->GetOutputPort();
    updateImageListIcons();

    this->visualizePage();
  } else {
    //    int ret = 
    QMessageBox::critical(this, tr("WSE"),
      tr("Endo image must be a binary segmentation (values [0,1])"),QMessageBox::Ok);
  }
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void wseGUI::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

void wseGUI::saveLayout()
 {
     QString fileName
         = QFileDialog::getSaveFileName(this, tr("Save layout"));
     if (fileName.isEmpty())
         return;
     QFile file(fileName);
     if (!file.open(QFile::WriteOnly)) {
         QString msg = tr("Failed to open %1\n%2")
                         .arg(fileName)
                         .arg(file.errorString());
         QMessageBox::warning(this, tr("Error"), msg);
         return;
     }

     QByteArray geo_data = saveGeometry();
     QByteArray layout_data = saveState();

     bool ok = file.putChar((uchar)geo_data.size());
     if (ok)
         ok = file.write(geo_data) == geo_data.size();
     if (ok)
         ok = file.write(layout_data) == layout_data.size();

     if (!ok) {
         QString msg = tr("Error writing to %1\n%2")
                         .arg(fileName)
                         .arg(file.errorString());
         QMessageBox::warning(this, tr("Error"), msg);
         return;
     }
 }

void wseGUI::loadLayout()
 {
     QString fileName
         = QFileDialog::getOpenFileName(this, tr("Load layout"));
     if (fileName.isEmpty())
         return;
     QFile file(fileName);
     if (!file.open(QFile::ReadOnly)) {
         QString msg = tr("Failed to open %1\n%2")
                         .arg(fileName)
                         .arg(file.errorString());
         QMessageBox::warning(this, tr("Error"), msg);
         return;
     }

     uchar geo_size;
     QByteArray geo_data;
     QByteArray layout_data;

     bool ok = file.getChar((char*)&geo_size);
     if (ok) {
         geo_data = file.read(geo_size);
         ok = geo_data.size() == geo_size;
     }
     if (ok) {
         layout_data = file.readAll();
         ok = layout_data.size() > 0;
     }

     if (ok)
         ok = restoreGeometry(geo_data);
     if (ok)
         ok = restoreState(layout_data);

     if (!ok) {
         QString msg = tr("Error reading %1")
                         .arg(fileName);
         QMessageBox::warning(this, tr("Error"), msg);
         return;
     }
 }

void wseGUI::writeSettings()
{
  g_settings->setValue("window_geometry", saveGeometry());
  g_settings->setValue("window_state",    saveState());
}

void wseGUI::readSettings()
{
  g_settings = new QSettings("cates", "WSE");

  // Restore window geometry
  if (g_settings->contains("window_geometry"))
   restoreGeometry(g_settings->value("window_geometry").toByteArray());

  // Restore window state (including dock states)
  if (g_settings->contains("window_state"))
    { restoreState(g_settings->value("window_state").toByteArray()); }
  
  if (g_settings->value("import_path").isNull()) { g_settings->setValue("import_path", tr(".")); }
  if (g_settings->value("export_path").isNull()) { g_settings->setValue("export_path", tr(".")); }
}


void wseGUI::visualizePage()
{
  // if (mImageData != -1) {
  //   mIsoRenderer->setImageData(mImageStack->image(mImageData));
  // } else {
  //   mIsoRenderer->setImageData(NULL);
  // }

  // if (mImageMask != -1) {
  //   mIsoRenderer->setWallData(mImageStack->image(mImageMask));
  // } else {
  //   mIsoRenderer->setWallData(NULL);
  // }

  // if (mIsosurfaceImage != -1) {
  //   mIsoRenderer->setEndoData(mImageStack->image(mIsosurfaceImage));
  // } else {
  //   mIsoRenderer->setEndoData(NULL);
  // }

  // SmoothSettings settings;
  // settings.mEnabled = ui.smoothGroupBox->isChecked();
  // settings.mAntiAliasing = ui.antiAliasCheckBox->isChecked();
  // settings.mGaussianBlurring = ui.gaussianBlurCheckBox->isChecked();
  // settings.mAntiAliasIterations = ui.antiAliasIterationsSlider->value();
  // settings.mGaussianVariance = ui.gaussianVarianceSlider->value() / 100.0f;

  // mIsoRenderer->setSmoothSettings(settings);
  // mIsoRenderer->setMeshSubdivision(ui.subdivideMeshCheckBox->isChecked());

  // mIsoRenderer->setSlice(mSliceViewer->GetFinalOutput());
  // mIsoRenderer->createIsoSurfaces();
  // updateColorMap(); 

}


void wseGUI::exportPage()
{
  //  if (mExportDirty)
  //   {
  //     // Update list widgets
  //     ui.exportClassListWidget->clear();

  //     ui.exportButton->setEnabled(false);
  //     mExportColormapAction->setEnabled(false);
  //     ui.exportDisplay->setBackground(QImage());
  //     mExportDirty = false;
  //   }

  //   ui.stackedWidget->setCurrentIndex(4);
}


void wseGUI::viewerChangeSlice()
{
  if (mImageData == -1) {
    return;
  }
  std::stringstream ss;
  ss << this->ui.sliceSelector->value();
  ui.sliceNumberLabel->setText(QString(ss.str().c_str()));
  this->mSliceViewer->SetZSlice(this->ui.sliceSelector->value());
  this->mSliceViewer->Render();


  mCrosshairActor->SetMapper(NULL);


  //  mIsoRenderer->setSliceDisplayExtent(mSliceViewer->GetImageActor()->GetDisplayExtent());
  // mIsoRenderer->updateClipPlane();
  // redrawIsoSurface();
}

void wseGUI::updateImageDisplay() {

  if (mImageData != -1) {
    mSliceViewer->SetImageMask(NULL);

    Image *image = mImageStack->image(mImageData);
    vtkImageImport *imageImport = image->originalVTK();
    mSliceViewer->SetInputConnection(imageImport->GetOutputPort());

    // Add the mask as an overlay to the image view window
    if (mImageMask != -1) {
      mSliceViewer->SetImageMask(mImageStack->image(mImageMask)->originalVTK()->GetOutputPort());
    }

    mSliceViewer->Render();

    // Set up the image slider
    ui.sliceSelector->setMinimum(0);
    ui.sliceSelector->setMaximum(image->nSlices()-1);
    ui.sliceSelector->setSingleStep(1);
    ui.sliceSelector->setPageStep(1);
  } else {
    // disable rendering pipeline
    //mVTKImageViewer2->SetInput(mNullVTKImageData);
    mSliceViewer->DisableDisplay();
  }

  QTime startTime =  QTime::currentTime();


  updateHistogram();

  qint32 msecs = startTime.msecsTo( QTime::currentTime() );
  std::cerr << "Histogram generation took " << msecs << "ms\n";

  visualizePage();
  setMouseHandling();

}

void wseGUI::on_imageListWidget_itemSelectionChanged()
//void wseGUI::importSelectionChanged()
{
  QList<QListWidgetItem *> items = ui.imageListWidget->selectedItems();
  if (items.size())
  {
    ui.sliceSelector->setEnabled(true);
    ui.deleteButton->setEnabled(true);
    mImageStack->setSelectedByName((items.at(0))->text());
    //    std::cout << items.at(0)->text().toAscii().constData()<<std::endl;   
  }
  else
  {
    ui.sliceSelector->setEnabled(false);
    ui.deleteButton->setEnabled(false);
    mImageStack->clearSelection();
  }
}

void wseGUI::importAdd()
{
  QStringList files = QFileDialog::getOpenFileNames(this, tr("Import Images"),
                                                    g_settings->value("import_path").toString(), 
                                                    tr("ImageFiles (*.tif *.png *.jpg *.nrrd *.dcm *.mhd *.mha)"));
  for (int i = 0; i < files.size(); i++)
  {
    QString imagePath = files.at(i);
    if (!addImageFromFile(imagePath))
      break;
  }
}

void wseGUI::importDelete()
{
  QList<QListWidgetItem *> items = ui.imageListWidget->selectedItems();
  for (int i = items.size()-1; i >=0; i--)
  {
    QListWidgetItem *item = items.at(i);
    int row = ui.imageListWidget->row(item);
    QString name = item->text();
    if (mImageStack->removeImage(name))
    {
      ui.imageListWidget->takeItem(row);
      delete item;
    }
    if (row == mImageData) {
      mImageData = -1;
      ui.sliceSelector->setEnabled(false);
      updateImageDisplay();
    } else if (row == mImageMask) {
      mImageMask = -1;
    } else if (row == mIsosurfaceImage) {
      mIsosurfaceImage = -1;
    }

    if (mImageData > row) {
      mImageData--;
    }
    if (mImageMask > row) {
      mImageMask--;
    }
    if (mIsosurfaceImage > row) {
      mIsosurfaceImage--;
    }

  }

  if (mImageStack->numImages() == 0)
  {
    mExportAction->setEnabled(false);
  }

  updateImageDisplay();

  this->populateImageDataComboBoxes();

  mExportDirty = true;   
}


void wseGUI::imageDropped(QString fname)
{
  addImageFromFile(fname);
}

bool wseGUI::addImageFromFile(QString fname)
{
  if (mImageStack->addImage(fname))
  {
    QListWidgetItem *item = new QListWidgetItem;

    item->setText(mImageStack->selectedName());
    ui.imageListWidget->insertItem(ui.imageListWidget->count(), item);
    ui.imageListWidget->setCurrentRow(ui.imageListWidget->count()-1);
    ui.setIsosurfaceButton->setEnabled(true);
    //    ui.setImageMaskButton->setEnabled(true);
    ui.setImageDataButton->setEnabled(true);
    mExportAction->setEnabled(true);
  }
  else
  {
    int ret = QMessageBox::warning(this, tr("WSE"),
                                   tr("There was an error loading image at $1!").arg(fname),
                                   QMessageBox::Ok);
    return ret;
    
  }
  QFileInfo fi(fname);
  QString path = fi.canonicalPath();
  if (!path.isNull())
    g_settings->setValue("import_path", path);
  
  this->populateImageDataComboBoxes();

  mExportDirty = true;
  return true;
}

void wseGUI::exportListItemChanged(QListWidgetItem *item)
{
  //  bool checked = false;
  //   int numEntries = ui.exportClassListWidget->count();
  //   ui.exportButton->setEnabled(checked);
  //   mExportColormapAction->setEnabled(checked);
}

void wseGUI::exportImage()
{
}

void wseGUI::visClassCheckAll()
{
  //  int numEntries = ui.visClassListWidget->count();
  // for (int i = 0; i < numEntries; i++)
  //  {
  //  ui.visClassListWidget->item(i)->setCheckState(Qt::Checked);
  // }
}


void wseGUI::visClassUncheckAll()
{
  //  int numEntries = ui.visClassListWidget->count();
  // for (int i = 0; i < numEntries; i++)
  // {
  //  ui.visClassListWidget->item(i)->setCheckState(Qt::Unchecked);
  // }
}


void wseGUI::visImageCheckAll()
{
  //  int numEntries = ui.visImageListWidget->count();
  // for (int i = 0; i < numEntries; i++)
  //{
  //  ui.visImageListWidget->item(i)->setCheckState(Qt::Checked);
  // }
}


void wseGUI::visImageUncheckAll()
{
  //  int numEntries = ui.visImageListWidget->count();
  //  for (int i = 0; i < numEntries; i++)
  //  {
  //    ui.visImageListWidget->item(i)->setCheckState(Qt::Unchecked);
  //  }
}


void wseGUI::exportCheckAll()
{
  //  int numEntries = ui.exportClassListWidget->count();
  //   for (int i = 0; i < numEntries; i++)
  //   {
  //     ui.exportClassListWidget->item(i)->setCheckState(Qt::Checked);
  //   }
}


void wseGUI::exportUncheckAll()
{
  //  int numEntries = ui.exportClassListWidget->count();
  //   for (int i = 0; i < numEntries; i++)
  //   {
  //     ui.exportClassListWidget->item(i)->setCheckState(Qt::Unchecked);
  //   }
}


void wseGUI::visSelectionChanged(QListWidgetItem *item)
{

}

void wseGUI::colorSchemeSelectorChanged(int val)
{
  //  mIsoRenderer->setBackgroundColor(mColorSchemes[val].background.r,
  //                               mColorSchemes[val].background.g,
  //                               mColorSchemes[val].background.b);

  //redrawIsoSurface();
}

void wseGUI::scalarMethodChanged(int m) {
  // mScalarMethod = m;
  //  mIsoRenderer->setScalarMethod(ScalarMethod::getScalarMethods()[m]);
  // mIsoRenderer->updateIsoScalars();
  //  redrawIsoSurface();
  // updateColorMap();
}


void wseGUI::visMethodChanged(int val)
{
}

void wseGUI::updateProgress(int p) {
  mProgressBar->setValue(p);
}

void wseGUI::updateHistogram() {
  std::cerr << "\nComputing histogram...\n";
  
  // If no image data is set, do not update histogram
  if (mImageData < 0) {
    return;
  }
  
  if (this->ui.numBinsSpinner->value() < mMinHistogramBins
      || this->ui.numBinsSpinner->value() > mMaxHistogramBins) {
    return;
  }

  int numBins = this->ui.numBinsSpinner->value();

  if (numBins <= 0) { 
    return;
  }
  
 
  delete mHistogram;


  Image::itkFloatImage::Pointer image = mImageStack->image(mImageData)->original();
  Image::itkFloatImage::Pointer mask(NULL);

  if (mImageMask >= 0) {
    mask = mImageStack->image(mImageMask)->original();
    mHistogram = new Histogram<Image::itkFloatImage>(image, mask, numBins);
  } else {
    mHistogram = new Histogram<Image::itkFloatImage>(image, numBins);
  }
    
  ui.lowerThresholdSpinBox->setRange(mHistogram->min(), mHistogram->max());
  ui.upperThresholdSpinBox->setRange(mHistogram->min(), mHistogram->max());

  std::cerr << "hist min = " << mHistogram->min() <<std::endl;
  std::cerr << "hist max = " << mHistogram->max() << std::endl;
  std::cerr << "hist mean = " << mHistogram->mean() <<std::endl;
  std::cerr << "hist stdev = " << mHistogram->stdev() << std::endl;
  std::cerr << "hist pixels = " << mHistogram->pixelCount() << std::endl;

  mPoints.clear();
  for (unsigned int bin = 0; bin < mHistogram->numBins(); bin++) {
    mPoints.push_back(mHistogram->frequencies()[bin]);
  }
 
  updateHistogramBars();
  updateHistogramWidget();

  if (mMarkers.size() > 0) {
    setThresholdToIntensity(mMarkers[0]);
  }
}

void wseGUI::updateHistogramBars() {

  // Image::itkFloatImage::Pointer image = mImageStack->image(mImageData)->original();
  // Image::itkFloatImage::Pointer mask(NULL);

  // if (mImageMask >= 0) {
  //   mask = mImageStack->image(mImageMask)->original();
  // }

  // mMarkers.clear();

  // if (mImageMask >= 0 && ui.histogramBarsComboBox->currentIndex() == 0) {

  //   std::vector<double> intensities;

  //   Quantification<Image::itkFloatImage> quantification (image, mask);
  //   quantification.calculateFibrosis(5, intensities, mPercentages);

  //   for (int i = 0; i < intensities.size(); ++i) {
  //     std::cerr << "fibrosis score #" << i+1 << ": " << std::fixed << std::setprecision(2) << mPercentages[i]*100;
  //     std::cerr << "% (intensity=" << intensities[i] << ")\n";
  //     mMarkers.push_back(intensities[i]);
  //   }
    
  // } else if (mImageMask >= 0 && ui.histogramBarsComboBox->currentIndex() == 1) {
      
  //     std::vector<double> intensities;
      
  //     Quantification<Image::itkFloatImage> quantification (image, mask);
  //     quantification.calculateScar(5, intensities, mPercentages);
      
  //     for (int i = 0; i < intensities.size(); ++i) {
  //       std::cerr << "scar score #" << i+1 << ": " << std::fixed << std::setprecision(2) << mPercentages[i]*100;
  //       std::cerr << "% (intensity=" << intensities[i] << ")\n";
  //       mMarkers.push_back(intensities[i]);
  //     }
      
  // } else {
  //   // basic stats 
  //   mMarkers.push_back(mHistogram->mean() - mHistogram->stdev()*2);
  //   mMarkers.push_back(mHistogram->mean() - mHistogram->stdev());
  //   mMarkers.push_back(mHistogram->mean());
  //   mMarkers.push_back(mHistogram->mean() + mHistogram->stdev());
  //   mMarkers.push_back(mHistogram->mean() + mHistogram->stdev()*2);
  // }

}

void wseGUI::updateHistogramWidget() {
  std::vector<int> points;
  std::vector<float> markers;
  int numBins = this->ui.numBinsSpinner->value();
  
  for (int i = 0; i < numBins; i++) {
    points.push_back(mPoints[i]);
  }

  for (unsigned int i = 0; i < mMarkers.size(); i++) {
    markers.push_back(mMarkers[i]);
  }
  
  ui.histogramSlider_1->setHistogram(points, markers, mHistogram->min(), mHistogram->max());
  ui.histogramSlider_1->update();
}


void wseGUI::thresholdChanged(double lower, double upper) {

  if (mHistogram == NULL) {
    return;
  }
  float min = mHistogram->min();
  float max = mHistogram->max();

//  fprintf (stderr, "min = %f, max = %f\n", m, m2);
  mThresholdLower = (max-min) * lower + min;
  mThresholdUpper = (max-min) * upper + min;
  
//  fprintf (stderr, "threshold is now %G percent, value:%G\n", val,threshold);
  mSliceViewer->SetThreshold(mThresholdLower, mThresholdUpper);
  mSliceViewer->Render();
  //  mIsoRenderer->setThreshold(mThresholdLower, mThresholdUpper);

  mThresholdTimer.stop();
  mThresholdTimer.start(100);
}


void wseGUI::on_showMaskCheckBox_stateChanged(int state)
{
  mSliceViewer->SetShowMask(state == Qt::Checked); 
  this->mSliceViewer->Render();
  // mIsoRenderer->setSlice(mSliceViewer->GetFinalOutput());
  // mIsoRenderer->updateSlice();
  // redrawIsoSurface();
}

void wseGUI::on_imageMaskOpacitySlider_valueChanged(int value)
{
  mSliceViewer->SetMaskOpacity(value / 100.0f);
  this->mSliceViewer->Render();
  redrawIsoSurface();
}

void wseGUI::on_showThresholdCheckBox_stateChanged(int state)
{
  mSliceViewer->SetShowThreshold(state == Qt::Checked);
  this->mSliceViewer->Render();
  //  mIsoRenderer->setSlice(mSliceViewer->GetFinalOutput());
  // mIsoRenderer->updateSlice();
  // redrawIsoSurface();
}

void wseGUI::on_thresholdOpacitySlider_valueChanged(int value)
{
  mSliceViewer->SetThresholdOpacity(value / 100.0f);
  this->mSliceViewer->Render();
  redrawIsoSurface();
}

void wseGUI::on_clipThresholdCheckBox_stateChanged(int state)
{
  mSliceViewer->SetClipThresholdToMask(state == Qt::Checked);
  this->mSliceViewer->Render();
}

void wseGUI::thresholdTimerEvent() {
  // mIsoRenderer->updateIsoScalars();
  updateColorMap();
  // redrawIsoSurface();
  reportThreshold();
}


void wseGUI::on_showNormalsCheckBox_stateChanged(int state) {
  //mIsoRenderer->setShowNormals(state == Qt::Checked);
  // mIsoRenderer->updateHedgeHogs();
  // redrawIsoSurface();
}





void wseGUI::on_antiAliasIterationsSlider_valueChanged(int value) {
  ui.antiAliasIterationsLabel->setText(QString("Iterations: " + QString::number(value)));
}
void wseGUI::on_gaussianVarianceSlider_valueChanged(int value) {
  QString var = QString::number(value / 100.0f);
  ui.gaussianVarianceLabel->setText(QString("Variance: " + var + QString(" * ") + var));
}



void wseGUI::redrawIsoSurface() {
  if (mImageData == -1 || mImageMask == -1 || mIsosurfaceImage == -1) {
    return;
  }

  // mIsoRenderer->redrawIsoSurface();
  ui.vtkRenderWidget->repaint();
}

void wseGUI::on_clipIsoSurfaceCheckBox_stateChanged( int value ) {
  //mIsoRenderer->setClipSurfaceToSlice(value == Qt::Checked);
  // redrawIsoSurface();
}

void wseGUI::on_showMaskOnIsoSurface_stateChanged( int value ) {
  // mIsoRenderer->setSliceVisibility(value == Qt::Checked);
  // redrawIsoSurface();
}



void wseGUI::on_clipIsoSurfaceComboBox_currentIndexChanged( int index ) {
  // mIsoRenderer->setClipDirection(index == 0);
  // mIsoRenderer->updateClipPlane();
  // redrawIsoSurface();
}




void wseGUI::toggleFullScreen() {
  if (mFullScreen) {
    showNormal();
  } else {
    showFullScreen();
  } 
  mFullScreen = !mFullScreen;
  mFullScreenAction->setChecked(mFullScreen);
}



void wseGUI::setDualView()
{
  // QList<int> controlSizes;
  // controlSizes.push_back(0);
  // controlSizes.push_back(100);
  // ui.controlSplitter->setSizes(controlSizes);

  QList<int> histSizes;
  histSizes.push_back(100);
  histSizes.push_back(0);
  ui.histogramSplitter->setSizes(histSizes);

  QList<int> viewSizes;
  viewSizes.push_back(ui.viewSplitter->width()/2);
  viewSizes.push_back(ui.viewSplitter->width()/2);
  ui.viewSplitter->setSizes(viewSizes);
}

void wseGUI::setNormalView()
{
  // QList<int> newSizes;
  // newSizes.push_back(20);
  // newSizes.push_back(100);
  // ui.controlSplitter->setSizes(newSizes);

  QList<int> histSizes;
  histSizes.push_back(100);
  histSizes.push_back(20);
  ui.histogramSplitter->setSizes(histSizes);

  QList<int> viewSizes;
  viewSizes.push_back(ui.viewSplitter->width()/2);
  viewSizes.push_back(ui.viewSplitter->width()/2);
  ui.viewSplitter->setSizes(viewSizes);
}


void wseGUI::setSliceView()
{
  // QList<int> controlSizes;
  // controlSizes.push_back(0);
  // controlSizes.push_back(100);
  // ui.controlSplitter->setSizes(controlSizes);

  QList<int> histSizes;
  histSizes.push_back(100);

  // Only display the histogram if there is actual image data
  if (mImageData != -1) { histSizes.push_back(20); }
  else { histSizes.push_back(0); }
  
ui.histogramSplitter->setSizes(histSizes);

  QList<int> viewSizes;
  viewSizes.push_back(100);
  viewSizes.push_back(0);
  viewSizes.push_back(0);
  ui.viewSplitter->setSizes(viewSizes);
}

void wseGUI::setIsoSurfaceView()
{
  //  QList<int> controlSizes;
  // controlSizes.push_back(0);
  // controlSizes.push_back(100);
  // ui.controlSplitter->setSizes(controlSizes);

  QList<int> histSizes;
  histSizes.push_back(100);
  histSizes.push_back(0);
  ui.histogramSplitter->setSizes(histSizes);

  QList<int> viewSizes;
  viewSizes.push_back(0);
  viewSizes.push_back(0);
  viewSizes.push_back(100);
  ui.viewSplitter->setSizes(viewSizes);
}



void wseGUI::on_endoSurfaceComboBox_currentIndexChanged( int index )
{
  // mIsoRenderer->setRenderMethods(ui.wallSurfaceComboBox->currentText().toStdString(), 
  //  ui.endoSurfaceComboBox->currentText().toStdString());
  // redrawIsoSurface();
}

void wseGUI::on_wallSurfaceComboBox_currentIndexChanged( int index )
{
  // mIsoRenderer->setRenderMethods(ui.wallSurfaceComboBox->currentText().toStdString(), 
  // ui.endoSurfaceComboBox->currentText().toStdString());
  // redrawIsoSurface();
}



void wseGUI::on_scalarSurfaceComboBox_currentIndexChanged( int index )
{
  if (index == 0) {
    // mIsoRenderer->setScalarOnWall(true);
    if (ui.wallSurfaceComboBox->currentIndex() == ISO_NOT_SHOWN) {
      ui.wallSurfaceComboBox->setCurrentIndex(ISO_SURFACE);
    }
  } else {
    // mIsoRenderer->setScalarOnWall(false);
    ui.wallSurfaceComboBox->setCurrentIndex(ISO_NOT_SHOWN);
    if (ui.endoSurfaceComboBox->currentIndex() == ISO_NOT_SHOWN) {
      ui.endoSurfaceComboBox->setCurrentIndex(ISO_SURFACE);
    }
  }
  visualizePage();
  // redrawIsoSurface();
}


void wseGUI::on_imageInterpolationComboBox_currentIndexChanged( int index )
{
  // mIsoRenderer->setLinearInterpolation(ui.imageInterpolationComboBox->currentIndex() == 1,
  //  ui.maskInterpolationComboBox->currentIndex() == 1); 
  // mIsoRenderer->updateIsoScalars();
  // redrawIsoSurface();
}

void wseGUI::on_maskInterpolationComboBox_currentIndexChanged( int index )
{
  // mIsoRenderer->setLinearInterpolation(ui.imageInterpolationComboBox->currentIndex() == 1,
  //   ui.maskInterpolationComboBox->currentIndex() == 1); 
  // mIsoRenderer->updateIsoScalars();
  //redrawIsoSurface();
}


void wseGUI::setThresholdToIntensity( double value ) {

  float min = mHistogram->min();
  float max = mHistogram->max();

  float lowerThreshold = (value - min) / (max-min);
  float upperThreshold = 100.0f;

  ui.histogramSlider_1->setLowerThreshold(lowerThreshold);
  ui.histogramSlider_1->setUpperThreshold(upperThreshold);
  thresholdChanged(ui.histogramSlider_1->getLowerThreshold(), ui.histogramSlider_1->getUpperThreshold());
  reportThreshold();
}

void wseGUI::setThresholdToIntensity( double lower, double upper )
{
  if (lower == mThresholdLower && upper == mThresholdUpper) {
    return;
  }

  float min = mHistogram->min();
  float max = mHistogram->max();

  float lowerThreshold = (lower - min) / (max-min);
  float upperThreshold = (upper - min) / (max-min);

  ui.histogramSlider_1->setLowerThreshold(lowerThreshold);
  ui.histogramSlider_1->setUpperThreshold(upperThreshold);
  thresholdChanged(ui.histogramSlider_1->getLowerThreshold(), ui.histogramSlider_1->getUpperThreshold());
  reportThreshold();
}


//void wseGUI::snapToFibrosisMarker( unsigned int num ) {
//   if (mMarkers.size() > num) {

//     std::stringstream ss;
//     ss << "Threshold set to intensity " << mMarkers[num] << " (" << 
//       std::fixed << std::setprecision(2) << mPercentages[num]*100 << "% of masked pixels)";

//     showStatusMessage(tr(ss.str().c_str()), 25000);

//     setThresholdToIntensity(mMarkers[num]);
//   }
//}



void wseGUI::reportThreshold()
{

  // if (mImageData == -1 || mImageMask == -1) {
  //   return;
  // }

 
  // float percentage = computePercentForIntensity(mThresholdLower, mThresholdUpper);

  // std::stringstream ss;
  // ss << "Threshold set to (" << mThresholdLower << ", " << mThresholdUpper << ") - " 
  //   << std::fixed << std::setprecision(2) << percentage << "% of masked pixels";

  // std::stringstream percentage_string;
  // percentage_string << std::fixed << std::setprecision(2) << percentage << "%";

  // mPercentageString = QString(percentage_string.str().c_str());

  // ui.lowerThresholdSpinBox->setValue(mThresholdLower);
  // ui.upperThresholdSpinBox->setValue(mThresholdUpper);

  // updatePercentageDisplay();

  // //showStatusMessage(tr(ss.str().c_str()), 25000);
}

void wseGUI::showStatusMessage( const QString &text, int timeout /*= 0*/ ) {
  std::cerr << text.toStdString() << std::endl;
  ui.statusBar->showMessage(text, timeout);
}

void wseGUI::on_smoothThresholdCheckBox_stateChanged(int value) {
  mSmoothStepThreshold = value == Qt::Checked;
  // mIsoRenderer->setSmoothThreshold(mSmoothStepThreshold);
  on_smoothThresholdSlider_valueChanged(ui.smoothThresholdSlider->value());
  mThresholdTimer.stop();
  mThresholdTimer.start(100);
}


void wseGUI::on_smoothThresholdSlider_valueChanged(int value) {

  // determine the ratio (0..1) of the slider 
  float ratio = value / 100.0f;

  // exponential
  float input = ratio * 1.3f;
  mSmoothThresholdWidth = input * input * input * input * input;

  // mIsoRenderer->setSmoothThreshold(mSmoothStepThreshold);
  // mIsoRenderer->setSmoothThresholdWidth(mSmoothThresholdWidth);
  //std::cerr << "Width = " << mSmoothThresholdWidth << "\n";

  // use a timer so that the ui has a more interactive response time
  if (mSmoothStepThreshold) {  // only if enabled  
    mThresholdTimer.stop();
    mThresholdTimer.start(100);
  }
}

void wseGUI::colorMapChanged( int m ) {
  mCurrentColorMap = m;
  updateColorMap();
}

void wseGUI::updateColorMap() {

 //  double numSteps = 5;

//   double minValue, maxValue;
// double scalarMax = mIsoRenderer->getScalarMax();
// double scalarMin = mIsoRenderer->getScalarMin();

//   if (ui.histogramColorScaleCheckBox->isChecked()) {
//     minValue = mHistogram->min();
//     maxValue = mHistogram->max();
//   } else {
//     minValue = scalarMin;
//     maxValue = scalarMax;
//   }

//   QwtDoubleInterval range(minValue, maxValue);
//   QwtLinearScaleEngine scaleEngine;

//   double stepSize = range.width() / numSteps;

// //  scaleEngine.autoScale(10,minValue,maxValue,stepSize);
//   QwtScaleDiv scaleDiv = scaleEngine.divideScale(minValue, maxValue, numSteps, 5, stepSize);

//   colorMap map = mColorMaps[mCurrentColorMap];
//   int numColors = map.colors.size();

//   mColorMap.setColorInterval(map.colors[0], map.colors[numColors-1]);

//   if (ui.snapColorsToBarsCheckBox->isChecked()) {
//     for (int i = 0; i < mMarkers.size(); i++) {
//       double ratio = (mMarkers[i] - minValue) / (maxValue - minValue);
//       if (i+1 < map.colors.size()) {
//         mColorMap.addColorStop(ratio, map.colors[i+1]);
//       }
//     }
//   } else {
//     double width = 1.0 / (numColors-1);
//     for (int i = 1; i < numColors-1; i++) {
//       mColorMap.addColorStop(i*width, map.colors[i]);
//     }
//   }

//   //mColorMap.addColorStop(0.75, Qt::yellow);
//   mScaleWidget.setColorBarEnabled(true);
//   mScaleWidget.setColorMap(range, mColorMap);

//   mScaleWidget.setScaleDiv(scaleEngine.transformation(), scaleDiv);

//   vtkLookupTable *lut = mIsoRenderer->getLookupTable();

//   lut->SetTableValue(0,0.4f,0.4f,0.4f,1.0f);
//   // AKM: For the grey non-threshold pictures
//   //lut->SetTableValue(1,0.25f,0.25f,0.25f,1.0f);

//   // AKM: For the grey non-threshold pictures
//   //for (int i = 2; i < 1000; i++ ) {
//   for (int i = 0; i < 1000; i++ ) {
//     double value = (i / 1000.0) * (scalarMax - scalarMin) + scalarMin;
//     QRgb rgb = mColorMap.rgb(range, value);
//     lut->SetTableValue(i+1, qRed(rgb)/255.0, qGreen(rgb)/255.0, qBlue(rgb)/255.0, 1.0f);
//   }
  

//   redrawIsoSurface();
 }

void wseGUI::on_histogramBarsComboBox_currentIndexChanged( int index ) {
  updateHistogramBars();
  updateHistogramWidget();
  updateColorMap();
}

void wseGUI::on_histogramColorScaleCheckBox_stateChanged( int ) {
  updateColorMap();
}

void wseGUI::on_snapColorsToBarsCheckBox_stateChanged( int ) {
  updateColorMap();
}


void wseGUI::on_smoothGroupBox_toggled( bool expanded )
{
  // show/hide direct children
  foreach(QObject* child, ui.smoothGroupBox->children()) {
    if (child->isWidgetType()) {
      static_cast<QWidget*>(child)->setVisible(expanded);
    }
  }
  ui.smoothGroupBox->setFlat(!expanded);
}

void wseGUI::on_advancedOptionsGroupBox_toggled( bool expanded )
{
  // show/hide direct children
  foreach(QObject* child, ui.advancedOptionsGroupBox->children()) {
    if (child->isWidgetType()) {
      static_cast<QWidget*>(child)->setVisible(expanded);
    }
  }
  ui.advancedOptionsGroupBox->setFlat(!expanded);
}

void wseGUI::toggleThresholdDisplay()
{
  if (ui.showThresholdCheckBox->checkState() == Qt::Checked) {
    ui.showThresholdCheckBox->setCheckState(Qt::Unchecked);
  } else {
    ui.showThresholdCheckBox->setCheckState(Qt::Checked);
  }
}


void wseGUI::togglePercentageShown()
{
  mPercentageShown = !mPercentageShown;

  updatePercentageDisplay();
}


void wseGUI::changeSlice( bool direction )
{
  int slice = this->ui.sliceSelector->value();
  if (direction) {
    slice++;
  } else {
    slice--;
  }
  ui.sliceSelector->setValue(slice);
}

void wseGUI::wheelEvent(QWheelEvent * event) 
{
  if (event->delta() > 0) {
    changeSlice(true);
  } else {
    changeSlice(false);
  }
}

void wseGUI::setMouseHandling()
{
  ui.vtkImageWidget->GetRenderWindow()->GetInteractor()->RemoveObservers(vtkCommand::MouseWheelBackwardEvent);
  ui.vtkImageWidget->GetRenderWindow()->GetInteractor()->RemoveObservers(vtkCommand::MouseWheelForwardEvent);
  ui.vtkImageWidget->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::MouseWheelBackwardEvent, mVTKCallback);
  ui.vtkImageWidget->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::MouseWheelForwardEvent, mVTKCallback);

  ui.vtkRenderWidget->GetRenderWindow()->GetInteractor()->RemoveObservers(vtkCommand::MouseWheelBackwardEvent);
  ui.vtkRenderWidget->GetRenderWindow()->GetInteractor()->RemoveObservers(vtkCommand::MouseWheelForwardEvent);
  ui.vtkRenderWidget->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::MouseWheelBackwardEvent, mVTKCallback);
  ui.vtkRenderWidget->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::MouseWheelForwardEvent, mVTKCallback);
}

void wseGUI::pointPick()
{
  double data[3];
  mPointPicker->GetSelectionPoint(data);

  double pickPosition[3];
  mPointPicker->GetPickPosition(pickPosition);

  std::cerr << "Pick: Selection Point = (" << data[0] << ", " << data[1] << ", " << data[2] << ")\n";
  std::cerr << "Pick: Position        = (" << pickPosition[0] << ", " << pickPosition[1] << ", " << pickPosition[2] << ")\n";

  int slice = mImageStack->image(mImageData)->getSliceForPoint(pickPosition);
  std::cerr << "Pick: slice: " << slice << "\n";
  ui.sliceSelector->setValue(slice);


  const int length = 3;
  double x = pickPosition[0];
  double y = pickPosition[1];
  //double z = pickPosition[2];
  double z = mImageStack->image(mImageData)->getClosestSlicePoint(pickPosition);
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(5);
  points->SetPoint(0, x, y, z);
  points->SetPoint(1, x+length, y, z);
  points->SetPoint(2, x, y+length, z);
  points->SetPoint(3, x-length, y, z);
  points->SetPoint(4, x, y-length, z);


  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

  for (int i=1; i<5; i++) {
    lines->InsertNextCell(2);
    lines->InsertCellPoint(0);
    lines->InsertCellPoint(i);
  }

  vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->SetName("Colors");
  colors->SetNumberOfComponents(3);
  colors->SetNumberOfTuples(5);
  for (int i = 0; i < 5 ;i++) {
    colors->InsertTuple3(i, 255, 255, 0);
  }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);
  polyData->GetPointData()->AddArray(colors);

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInput(polyData);

  mapper->ScalarVisibilityOn();
  mapper->SetScalarModeToUsePointFieldData();
  mapper->SelectColorArray("Colors");
 
  mCrosshairActor->SetMapper(mapper);

  mSliceViewer->Render();
}

//void wseGUI::on_lowerThresholdSpinBox_valueChanged( double value )
//{
//  setThresholdToIntensity(value, mThresholdUpper);
//}

//void wseGUI::on_upperThresholdSpinBox_valueChanged( double value )
//{
//  setThresholdToIntensity(mThresholdLower, value);
//}



// float wseGUI::computePercentForIntensity( float filter_min, float filter_max )
// {
//   float percentage;
//   unsigned long pixelCount;
//   computePercentForIntensity(filter_min, filter_max, percentage, pixelCount);
//   return percentage;
// }

// void wseGUI::computePercentForIntensity( float filter_min, float filter_max, float &percentage, unsigned long &pixelCount )
// {
//   int mask_pixels = mHistogram->pixelCount();


//   typedef Image::itkFloatImage ImageType;
//   typedef itk::ImageRegionConstIterator<ImageType> ConstIteratorType;
//   typedef ImageType::PixelType PixelType;

//   ImageType::Pointer image = mImageStack->image(mImageData)->original();
//   ImageType::Pointer mask = mImageStack->image(mImageMask)->original();

//   ImageType::SizeType image_size = image->GetLargestPossibleRegion().GetSize();
//   ImageType::SizeType mask_size = mask->GetLargestPossibleRegion().GetSize();


//   ConstIteratorType mit(mask, mask->GetRequestedRegion());
//   ConstIteratorType it(image, image->GetRequestedRegion());
//   it.GoToBegin();
//   mit.GoToBegin();
//   pixelCount = 0;
//   double sum = 0;
//   for( ; !it.IsAtEnd(); ++it)
//   {
//     PixelType pixel = it.Get();
//     if ((mask_size[0] == 0 || mit.Get() != 0) &&
//       ((pixel > filter_min && pixel < filter_max))) 
//     {
//       pixelCount++;
//     }
//     if (mask_size[0] != 0) ++mit;
//   }



//   percentage = (float)pixelCount / (float)mask_pixels * 100.0f;
// }







// float wseGUI::cmdLineComputeIntensity( std::string data, std::string mask, float percent )
// {
//   addImageFromFile(QString(data.c_str()));
//   addImageFromFile(QString(mask.c_str()));

//   mImageData = 0;
//   mImageMask = 1;

//   updateImageDisplay();

//   //std::cout << "total pixel count    : " <<  mHistogram->pixelCount() << "\n";
//   std::cout << mHistogram->pixelCount() << ";";

//   float intensity = mHistogram->max();
//   float computed_percent = 0.0f;
//   unsigned long pixel_count;
//   while (computed_percent < percent && intensity > mHistogram->min()) {
//     intensity--;
//     computePercentForIntensity(intensity, mHistogram->max(), computed_percent, pixel_count);
//   }

//   std::cout << pixel_count << ";";


//   return intensity;
// }

// float wseGUI::cmdLineComputePercentage( std::string data, std::string mask, float intensity )
// {
//   addImageFromFile(QString(data.c_str()));
//   addImageFromFile(QString(mask.c_str()));

//   mImageData = 0;
//   mImageMask = 1;

//   updateImageDisplay();


//   std::cout << mHistogram->pixelCount() << ";";

//   float computed_percent = 0.0f;
//   unsigned long pixel_count;
//   computePercentForIntensity(intensity, mHistogram->max(), computed_percent, pixel_count);
//   std::cout << pixel_count << ";";

//   return computed_percent;
// }

void wseGUI::updatePercentageDisplay()
{
  if (mPercentageShown) {
    ui.percentageLineEdit->setEnabled(true);
    ui.percentageLineEdit->setText(mPercentageString);
  } else {
    ui.percentageLineEdit->setEnabled(false);
    ui.percentageLineEdit->setText("Show with " + QString(QKeySequence(Qt::CTRL + Qt::Key_S)));
  }
}

void wseGUI::on_gaussianRadioButton_toggled(bool on)
{

  if (on == true)
    {
      // Show the gaussian controls
      ui.gaussianBlurringParamsBox->show();
    }
  else
    {
      // Hide the gaussian controls
      ui.gaussianBlurringParamsBox->hide();
    }

}

void wseGUI::on_anisotropicRadioButton_toggled(bool on)
{
  if (on == true)
    {
      // Show the anisotropic controls
      ui.anisotropicDiffusionParamsBox->show();
    }
  else
    {
      // Hide the anisotropic controls
      ui.anisotropicDiffusionParamsBox->hide();
    }
}

void wseGUI::on_curvatureRadioButton_toggled(bool on)
{
  if (on == true)
    {
      // Show the anisotropic controls
      ui.anisotropicDiffusionParamsBox->show();
    }
  else
    {
      // Hide the anisotropic controls
      ui.anisotropicDiffusionParamsBox->hide();
    }
}

} // end namespace
