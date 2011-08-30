#include "wse.h"

namespace wse {

QSettings *wseGUI::g_settings;

/** */
class InteractorCallback : public vtkCommand 
{
private:
  wseGUI *wse_;

public:
  InteractorCallback(wseGUI *wse) 
  {    wse_ = wse;  }

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
  {    wse_ = wse;  }
  
  virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData) 
  {    wse_->pointPick();  }
};
  
wseGUI::wseGUI(QWidget *parent, Qt::WFlags flags) : 
  QMainWindow(parent, flags),
  mWelcomeUrl("http://www.sci.utah.edu/~cates/wse"),
  mHelpUrl("https://github.com/joshcates/wse/wiki/Documentation"),
  mImageStack(NULL),
  mSegmentation(NULL),
  mHistogram(NULL),
  mMinHistogramBins(10),
  mMaxHistogramBins(1000),
  mCurrentColorMap(0),
  mImageData(-1),
  mImageMask(-1),
  mIsosurfaceImage(-1),
  mFullScreen(false)
{
  // TODO: Not used?
  mNullVTKImageData = vtkImageData::New();

  // Create the slice-by-slice image viewers
  mSliceViewer = SliceViewer::New();
  mSegmentSliceViewer = SegmentationViewer::New();
  mSegmentSliceViewer->SetImageMask(NULL);

  // Create the QThread objects that execute filtering
  mITKFilteringThread  = new 
    itk::QThreadITKFilter<itk::ImageToImageFilter<FloatImage::itkImageType,FloatImage::itkImageType> >;
  mITKSegmentationThread =  new 
    itk::QThreadITKFilter<itk::ImageToImageFilter<FloatImage::itkImageType,ULongImage::itkImageType> >;

  // Create key member variables
  mImageStack = new FloatImageStack();

#ifdef WIN32
  RedirectIOToConsole();
#endif

  // Setup main UI
  ui.setupUi(this);

  // Load window and layout settings
  this->readSettings();
  
  // Finish setting up UI
  this->setupUI();

  // Change the view to the standard slice view
  this->setSliceView();

  // Register image selection combo box widgets to stay in sync with file names
  mRegisteredImageComboBoxes.push_back(ui.denoisingInputComboBox);
  mRegisteredImageComboBoxes.push_back(ui.watershedInputComboBox);
  mRegisteredImageComboBoxes.push_back(ui.gradientInputComboBox);

  // Connect multithreading slots with signals
  connect(mITKFilteringThread,SIGNAL(finished()),this,SLOT(mITKFilteringThread_finished()));
  connect(mITKFilteringThread,SIGNAL(started()),this,SLOT(mITKFilteringThread_started()));
  connect(mITKSegmentationThread,SIGNAL(finished()),this,SLOT(mITKSegmentationThread_finished()));
  connect(mITKSegmentationThread,SIGNAL(started()),this,SLOT(mITKSegmentationThread_started()));
}

wseGUI::~wseGUI()
{
  delete mITKFilteringThread;
  delete mITKSegmentationThread;
  delete mSegmentation;

  if (mImageStack) { delete mImageStack; }
  //mVTKImageViewer->Delete();
  mSliceViewer->Delete();
  mNullVTKImageData->Delete();
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
  // Set up the browser
  ui.browserWindow->load(mWelcomeUrl);
  ui.consoleDockWidget->show(); // force a raise of this widget on start
  //  ui.browserWindow->show();
  //  ui.browserDockWidget->setWindowFlags(Qt::Drawer|Qt::RightDockWidgetArea);
  //  ui.browserDockWidget->show();

  // set up the isosurface renderer first
  // mIsoRenderer = new IsoRenderer();
  // ui.vtkRenderWidget->GetRenderWindow()->AddRenderer(mIsoRenderer->getSurfaceRenderer());
  // ui.vtkRenderWidget->show();
  
  ui.progressBar->hide();

  // Disable the flood level sliders to star
  ui.floodLevelA->setEnabled(false);
  ui.floodLevelB->setEnabled(false);

  // For now we only support a single segmentation at a time, so hide the list of segmentation data.
  ui.segmentationDataGroupBox->hide();

  // Create and initialize the picker objects
  PickerCallback *mPickerCallback = new PickerCallback(this);
  mPointPicker = vtkPointPicker::New();
  mPointPicker->AddObserver(vtkCommand::EndPickEvent, mPickerCallback);
  //  ui.vtkRenderWidget->GetInteractor()->SetPicker(mPointPicker);
  ui.vtkSegmentationWidget->GetInteractor()->SetPicker(mPointPicker);


  // Construct the Toolbar
  // mImportAction = new QAction(QIcon(":/WSE/Resources/import.png"), tr("&Load Volume"), this);
  // mImportAction->setToolTip(tr("Load an image volume"));
  // mImportAction->setStatusTip(tr("Load an image volume"));
  // mImportAction->setCheckable(true);
  // connect(mImportAction, SIGNAL(triggered()), this, SLOT(on_addButton_released()));

  // mExportAction = new QAction(QIcon(":/WSE/Resources/export.png"), tr("&Export"), this);
  // mExportAction->setToolTip(tr("Export"));
  // mExportAction->setStatusTip(tr("Export model"));
  // mExportAction->setCheckable(true);

  //  QActionGroup *toolBarGroup = new QActionGroup(this);
  //  toolBarGroup->addAction(mImportAction);
  // toolBarGroup->addAction(mExportAction);
  //  mImportAction->setChecked(false);
  // mExportAction->setEnabled(false);

  // ui.mainToolBar->addAction(mImportAction);
  // ui.mainToolBar->addAction(mExportAction);

  //  ui.mainToolBar->addAction(mImportAction);
  ui.mainToolBar->setIconSize(QSize(30,30));

  // HIDE TOOLBAR FOR NOW
  ui.mainToolBar->hide();
  ui.mainToolBar->setEnabled(false);
  
  this->setupUIMenu();

  this->setupUIDataManager();

   // Set up the viewers 
  ui.vtkImageWidget->SetRenderWindow(mSliceViewer->GetRenderWindow());
  mSliceViewer->SetupInteractor(ui.vtkImageWidget->GetRenderWindow()->GetInteractor());

  ui.vtkSegmentationWidget->SetRenderWindow(mSegmentSliceViewer->GetRenderWindow());
  mSegmentSliceViewer->SetupInteractor(ui.vtkSegmentationWidget->GetRenderWindow()->GetInteractor());


 // Set up the slice selection widgets and connections
  ui.sliceSelector->setEnabled(false);

  connect(ui.sliceSelector,SIGNAL(valueChanged(int)), this, SLOT(viewerChangeSlice()));
  ui.sliceSelector->setTracking(true);

  // 
  mVTKCallback = new InteractorCallback(this);

  // Set up mouse event handling 
  this->setMouseHandling();

  // Default window/level for the slice viewers and show the slice viewers
  mSliceViewer->SetColorLevel(128);
  mSliceViewer->SetColorWindow(256);
  ui.vtkImageWidget->show();

  mSegmentSliceViewer->SetColorLevel(256);
  mSegmentSliceViewer->SetColorWindow(256);
  ui.vtkSegmentationWidget->show();

  // Connect the visualization selection buttons
  connect(ui.setIsosurfaceButton, SIGNAL(released()), this, SLOT(setIsosurface()));
  connect(ui.setImageMaskButton,  SIGNAL(released()), this, SLOT(setImageMask()));
 
  // ... but disable them for now
  ui.setIsosurfaceButton->setEnabled(false);
  ui.setIsosurfaceButton->hide();
  ui.setImageDataButton->setEnabled(false);
  ui.setImageMaskButton->setEnabled(false);
  ui.setImageMaskButton->hide();

  // Initialize the color schemes
  for (unsigned int i = 0; i < mColorSchemes.size(); i++)
  {
    ui.colorSchemeSelector->addItem(QString(mColorSchemes[i].name.c_str()));
  }
  connect(ui.colorSchemeSelector, SIGNAL(currentIndexChanged(int)), this, 
    SLOT(colorSchemeSelectorChanged(int)));
                    
  // Set up histogram bins, window, and level and controls (along with connections).
  ui.numBinsSpinner->setMinimum(mMinHistogramBins);
  ui.numBinsSpinner->setMaximum(mMaxHistogramBins);
  ui.numBinsSpinner->setValue(mMaxHistogramBins/4);
  connect(ui.numBinsSpinner, SIGNAL(valueChanged(int)),this, SLOT(numBinsSpinnerChanged(int)));
  connect(ui.histogramSlider_1, SIGNAL(thresholdChanged(double, double)),this,SLOT(histogramThresholdChanged(double,double)));

  // Some conservative threshold values for WS filter
  ui.histogramSlider_1->setLowerThreshold(.10);
  ui.histogramSlider_1->setUpperThreshold(.40);



  for (unsigned int i = 0; i < mColorMaps.size(); ++i) {
    ui.colorMapComboBox->addItem(QString(mColorMaps[i].name.c_str()));
  }
  connect(ui.colorMapComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(colorMapChanged(int)));


  // Status Bar
  mProgressBar = new QProgressBar(this);
  mProgressBar->setRange(0,100);
  ui.statusBar->addPermanentWidget(mProgressBar);
  mProgressBar->hide();
  
  // ???
  this->updateColorMap(); 

  //  QVBoxLayout *vbox = new QVBoxLayout;
  // vbox->addWidget(&mScaleWidget);
  // vbox->setSpacing(0);
  // vbox->setMargin(0);
  //  ui.colorScaleGroupBox->setLayout(vbox);

  // Add crosshair actor to the slice viewer
  mCrosshairActor = vtkActor::New();
  mSliceViewer->GetRenderer()->AddActor(mCrosshairActor);

  // set defaults for basic mode
  //ui.scalarMethodComboBox->setCurrentIndex(scalarMethods.size()-1);
  ui.smoothGroupBox->setChecked(false);
  ui.advancedOptionsGroupBox->setChecked(false);
  ui.showMaskOnIsoSurface->setChecked(false);
  ui.scalarMethodComboBox->setCurrentIndex(1);
  ui.imageInterpolationComboBox->setCurrentIndex(1);
  ui.maskInterpolationComboBox->setCurrentIndex(1);
  ui.subdivideMeshCheckBox->setChecked(true);

  // Now initialize the Waterhshed segmentation module UI
  this->setupUIWatershedWindow();
}

void wseGUI::setupUIMenu()
{
  // Menubar -- Set up the actions, connect to slots, and create the menu
  // Load volume action
  mImportImageAction = new QAction(tr("Load Volume"), this);
  mImportImageAction->setShortcut(tr("Load image volumes from file"));
  connect(mImportImageAction, SIGNAL(triggered()),this,SLOT(on_addButton_released()));

  // Save volume action
  mExportImageAction = new QAction(tr("Save Volume"), this);
  connect(mExportImageAction, SIGNAL(triggered()),this,SLOT(on_saveImageButton_released()));
 
  // mExportColormapAction = new QAction(tr("Export Colormap"), this);
  // mExportColormapAction->setShortcut(tr("Export selected colormaps"));
  // mExportColormapAction->setEnabled(false);
  // connect(mExportColormapAction, SIGNAL(triggered()), this, SLOT(exportImage()));


  // Full screen action
  mFullScreenAction = new QAction(tr("&Full Screen"), this);
  mFullScreenAction->setShortcut(tr("Ctrl+F"));
  mFullScreenAction->setCheckable(true);
  connect(mFullScreenAction, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

  // Normal view action
  mNormalView = new QAction(tr("Go to &Normal View"), this);
  mNormalView->setShortcut((tr("Ctrl+1")));
  connect(mNormalView, SIGNAL(triggered()), this, SLOT(setNormalView()));

  // Dual view action
  mDualView = new QAction(tr("Go to &Dual View"), this);
  mDualView->setShortcut((tr("Ctrl+2")));
  connect(mDualView, SIGNAL(triggered()), this, SLOT(setDualView()));

  // Slice view action
  mSliceView = new QAction(tr("Go to &Slice View"), this);
  mSliceView->setShortcut((tr("Ctrl+3")));
  connect(mSliceView, SIGNAL(triggered()), this, SLOT(setSliceView()));

  // Isosurface action
  mIsoSurfaceView = new QAction(tr("Go to &IsoSurface View"), this);
  mIsoSurfaceView->setShortcut((tr("Ctrl+4")));
  connect(mIsoSurfaceView, SIGNAL(triggered()), this, SLOT(setIsoSurfaceView()));

  // Preferences action
  QAction *prefAction = new QAction(tr("&Preferences..."),this);
  prefAction->setStatusTip(tr("Set program preferences"));
  //  connect(prefAction,SIGNAL(triggered()), mPreferencesWindow,SLOT(exec()));  

  // Exit program action
  QAction *exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+Q"));
  exitAction->setStatusTip(tr("Exit the application"));
  connect(exitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

 // Construct file menu
  QMenu *fileMenu = ui.menuBar->addMenu(tr("&File"));
  fileMenu->addAction(mImportImageAction);
  fileMenu->addAction(mExportImageAction);
  fileMenu->addAction(prefAction);
  fileMenu->addAction(exitAction);
 
  // QMenu *toolsMenu = ui.menuBar->addMenu(tr("&Tools"));
  // toolsMenu->addAction(mImportAction);
  // toolsMenu->addAction(mExportAction);

  // View menu
  mViewControlWindowAction = new QAction(tr("View Control Window"), this);
  connect(mViewControlWindowAction, SIGNAL(triggered()), ui.controlsDockWidget, SLOT(show()));

  mViewWatershedWindowAction = new QAction(tr("View Watershed Window"), this);
  connect(mViewWatershedWindowAction, SIGNAL(triggered()), ui.watershedDockWidget, SLOT(show()));

  mViewDataWindowAction = new QAction(tr("View Data Window"), this);
  connect(mViewDataWindowAction, SIGNAL(triggered()), ui.dataDockWidget, SLOT(show()));

  mViewConsoleWindowAction = new QAction(tr("View Console"), this);
  connect(mViewConsoleWindowAction, SIGNAL(triggered()), ui.consoleDockWidget, SLOT(show()));
  
  QMenu *viewMenu = ui.menuBar->addMenu(tr("&View"));
  viewMenu->setTearOffEnabled(true);
  viewMenu->setSeparatorsCollapsible(true);
  viewMenu->addAction(mFullScreenAction);
  viewMenu->addAction(mNormalView);
  viewMenu->addAction(mDualView);
  viewMenu->addAction(mSliceView);
  viewMenu->addAction(mIsoSurfaceView);
  viewMenu->addSeparator();
  viewMenu->addAction(mViewDataWindowAction);
  viewMenu->addAction(mViewWatershedWindowAction);
  viewMenu->addAction(mViewConsoleWindowAction);
  //  viewMenu->addAction(mViewControlWindowAction);

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

  //
  mHelpAction = new QAction(tr("Help"),this);
  connect(mHelpAction, SIGNAL(triggered()), this, SLOT(displayHelp()));
  QMenu *helpMenu = ui.menuBar->addMenu(tr("&Help"));
  helpMenu->addAction(mHelpAction);

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

}

void wseGUI::setupUIDataManager()
{
 // Set up the Image List widget and connections-- this is the list
  // of loaded image volumes in the Data window
  ui.deleteButton->setEnabled(false);
  //  ui.imageListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui.imageListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  ui.imageListWidget->setDragEnabled(true);
  ui.imageListWidget->setAcceptDrops(true);
  ui.imageListWidget->setDropIndicatorShown(true);
  ui.imageListWidget->setDragDropMode(QAbstractItemView::DropOnly);
  ui.imageListWidget->setStatusTip(tr("Select the '+' icon or drag and drop images"));

  connect(ui.deleteButton, SIGNAL(released()), this, SLOT(importDelete()));

  connect(ui.imageListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, 
          SLOT(on_setImageDataButton_released()));
    
  connect(ui.imageListWidget, SIGNAL(imageDropped(QString)),
          this, SLOT(imageDropped(QString)));
}

void wseGUI::setupUIWatershedWindow()
{
  // Denoising UI setup
  ui.curvatureRadioButton->setChecked(true);
  ui.gaussianBlurringParamsBox->hide();

  //
}

void wseGUI::displayHelp()
{
  ui.consoleDockWidget->show();
  ui.consoleTabWidget->setCurrentIndex(1);
  ui.browserWindow->load(mHelpUrl);
}


void wseGUI::numBinsSpinnerChanged(int n) 
{
  this->updateHistogram();
}
  
  
void wseGUI::syncRegisteredImageComboBoxes()
{
  // Loop through all registered combo boxes (those in a member variable list)
  for (unsigned int j = 0; j < mRegisteredImageComboBoxes.size(); j++)
    {
      // Clear all entries
      mRegisteredImageComboBoxes[j]->clear();

      // Populate with the names in the image list widget
      for (int i=0; i<ui.imageListWidget->count(); i++)
	{
	  mRegisteredImageComboBoxes[j]->addItem(ui.imageListWidget->item(i)->text());
	}
      mRegisteredImageComboBoxes[j]->setCurrentIndex(ui.imageListWidget->count()-1);
    }
}
  
void wseGUI::syncRegisteredSegmentationComboBoxes()
{
  // Loop through all registered combo boxes (those in a member variable list)
  for (unsigned int j = 0; j < mRegisteredSegmentationComboBoxes.size(); j++)
    {
      // Clear all entries
      mRegisteredSegmentationComboBoxes[j]->clear();
      // Populate with the names in the segmentation list widget
      for (int i=0; i< ui.segmentationListWidget->count(); i++)
	{
	  mRegisteredSegmentationComboBoxes[j]->addItem(ui.segmentationListWidget->item(i)->text());
	}
      //      mRegisteredComboBoxes[j].setCurrentIndex(0);
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
  //  ui.setImageMaskButton->setEnabled(true);
  ui.sliceSelector->setEnabled(true);
  this->updateImageListIcons();
  this->updateImageDisplay();
  
  if (mSegmentation == NULL)
    {
      this->setSliceView();
    }
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

    //    this->visualizePage();
  } else {
    //    int ret = 
    QMessageBox::critical(this, tr("WSE"),
      tr("Endo image must be a binary segmentation (values [0,1])"),QMessageBox::Ok);
  }
}

void wseGUI::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
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

  // Restore histogram settings
  //  if (g_settings->value("histogram_low_thresh").isNull()) {g_settings->setValue("histogram_low_thresh",

}


void wseGUI::viewerChangeSlice()
{
  if (mImageData == -1)  { return;   }

  std::stringstream ss;
  ss << this->ui.sliceSelector->value();
  ui.sliceNumberLabel->setText(QString(ss.str().c_str()));
  
  // Change the slice number in the floating point image viewer
  this->mSliceViewer->SetZSlice(this->ui.sliceSelector->value());
  this->mSliceViewer->Render();

  // Change the slice number of the segmentation image 
  if (mSegmentation != NULL)
    {
      this->mSegmentSliceViewer->SetZSlice(this->ui.sliceSelector->value());
      this->mSegmentSliceViewer->Render();
    }

  mCrosshairActor->SetMapper(NULL);

  // mIsoRenderer->setSliceDisplayExtent(mSliceViewer->GetImageActor()->GetDisplayExtent());
  // mIsoRenderer->updateClipPlane();
  // redrawIsoSurface();
}

void wseGUI::updateImageDisplay() 
{
  if (mImageData != -1) 
    {
      mSliceViewer->SetImageMask(NULL);
      
      // Connect selected image to the viewer port
      mSliceViewer->SetInputConnection(mImageStack->image(mImageData)->vtkImporter()->GetOutputPort());
      
      // Connect segmentation to the segmentation viewer port if it
      // exists and its number of slices matches those of the floating
      // point volume.  This logic assumes that if a user is viewing a
      // volume with the same number of slices, then it could be an
      // image from which the segmentation was derived.  (If not, we
      // will at least still be safe displaying slices of the
      // segmentation.)
      if (mSegmentation != NULL)
	{
          if (mSegmentation->nSlices() == mImageStack->image(mImageData)->nSlices())
            {
              mSegmentSliceViewer->SetImageLookupTable(mSegmentation->GetLookupTable());

              //NOTE: SetInputConnection should be called AFTER
              //SetInputLookupTable.  SetInputConnection triggers a
              //call to UpdateDisplay.
              mSegmentSliceViewer->SetInputConnection(mSegmentation->GetOutputPort());
              this->mSegmentSliceViewer->SetZSlice(this->ui.sliceSelector->value());

              // TODO: Somehow rendering here causes two invalid
              // drawable errors. If I wait and let the change slice
              // events trigger re-rendering, then I do not get
              // errors.  Need to figure this out...
              // mSegmentSliceViewer->Render();
            }	  
	}
      else
        {  mSegmentSliceViewer->DisableDisplay();    }
      
      // Add the mask as an overlay to the image view window
      if (mImageMask != -1) 
	{
	  mSliceViewer->SetImageMask(mImageStack->image(mImageMask)->vtkImporter()->GetOutputPort());
	}
      mSliceViewer->Render();

    // Set up the image slider
    ui.sliceSelector->setMinimum(0);
    //    ui.sliceSelector->setMaximum(image->nSlices()-1);
    ui.sliceSelector->setMaximum(mImageStack->image(mImageData)->nSlices()-1);
    ui.sliceSelector->setSingleStep(1);
    ui.sliceSelector->setPageStep(1);
    } 
  else 
    {
      // disable rendering pipeline
      //mVTKImageViewer2->SetInput(mNullVTKImageData);
      mSliceViewer->DisableDisplay();
      mSegmentSliceViewer->DisableDisplay();
    }
  
  //  QTime startTime =  QTime::currentTime();
  this->updateHistogram();

  //  qint32 msecs = startTime.msecsTo( QTime::currentTime() );
  //  std::cerr << "Histogram generation took " << msecs << "ms\n";

  //  visualizePage();
  this->setMouseHandling();
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

void wseGUI::on_saveImageButton_released()
{
  if (mImageData == -1) 
    {
    QMessageBox::warning(this, tr("WSE"),
                                   tr("Please select an image volume to save."),
                                   QMessageBox::Ok);
    return;
    }

  // TODO: Suffix recognition is not working properly with
  // getSaveFileName.  Could this be a problem with Qt? I might need
  // to manually create a QFileDialog and set it up on my own
  QString filter = "Volumes (*.nrrd *.dcm *.mhd *.mha)";
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Volume"),
						  g_settings->value("export_path").toString(), 
						  filter, &filter,0);
  
  if (fileName.isEmpty()) return;

  this->output(QString("Saving volume file ") + fileName);


  bool ans;
  QString errStr;

  try 
    {
      ans = mImageStack->image(mImageData)->write(fileName);
    }
  catch (itk::ExceptionObject &e)
    {
      ans = false;
      errStr = e.GetDescription();
    }
    
  if (ans == false)
    {
    QMessageBox::warning(this, tr("WSE"),
                                   tr("Failed to save image volume.\n" + errStr),
                                   QMessageBox::Ok);
    }


  QFileInfo fi(fileName);
  QString path = fi.canonicalPath();
  if (!path.isNull()) {  g_settings->setValue("export_path", path); }


}

void wseGUI::on_addButton_released()
{
  QStringList files = QFileDialog::getOpenFileNames(this, tr("Import Images"),
                                                    g_settings->value("import_path").toString(), 
                                                    tr("Volumes (*.nrrd *.dcm *.mhd *.mha)"));
  for (int i = 0; i < files.size(); i++)
  {
    QString imagePath = files.at(i);
    if ( ! addImageFromFile(imagePath) ) { break; }
  }

  // Switch view to the last image loaded
  this->on_setImageDataButton_released();
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


      if (row == mImageData) 
	{
	  mImageData = -1;
	  ui.sliceSelector->setEnabled(false);
	  updateImageDisplay();
	} 
      else if (row == mImageMask) 
	{
	  mImageMask = -1;
	} 
      else if (row == mIsosurfaceImage) 
	{
	  mIsosurfaceImage = -1;
	}
      
      if (mImageData > row) 
	{
	  mImageData--;
	}
      if (mImageMask > row) 
	{
	  mImageMask--;
	}
      if (mIsosurfaceImage > row) 
	{
	  mIsosurfaceImage--;
	}     
    }
  
  if (mImageStack->numImages() == 0)
    {
      //    mExportAction->setEnabled(false);
    }
  updateImageDisplay();
  
  this->syncRegisteredImageComboBoxes();
}


void wseGUI::imageDropped(QString fname)
{
  addImageFromFile(fname);
}

bool wseGUI::addImageFromData(FloatImage *img)
{
  if (mImageStack->addImage(img))
  {
    QListWidgetItem *item = new QListWidgetItem;

    item->setText(mImageStack->selectedName());
    ui.imageListWidget->insertItem(ui.imageListWidget->count(), item);
    ui.imageListWidget->setCurrentRow(ui.imageListWidget->count()-1);
    //    ui.setIsosurfaceButton->setEnabled(true);
    //    ui.setImageMaskButton->setEnabled(true);
    ui.setImageDataButton->setEnabled(true);
  }
  else
  {
    int ret = QMessageBox::warning(this, tr("WSE"),
                                   tr("There was an error adding image from data."),
                                   QMessageBox::Ok);
    return ret;
    
  }
  this->syncRegisteredImageComboBoxes();

  return true;
}


bool wseGUI::addImageFromFile(QString fname)
{
  this->output(QString("Loading volume from ") + fname);

  bool ans = true;  
  QString errStr;
  try 
    {
      ans = mImageStack->addImage(fname);
    }
  catch(itk::ExceptionObject &e)
    {
      errStr = e.GetDescription();
      ans = false;
    }

  if (ans == true)
    {
      QListWidgetItem *item = new QListWidgetItem;
      
      item->setText(mImageStack->selectedName());
      ui.imageListWidget->insertItem(ui.imageListWidget->count(), item);
      ui.imageListWidget->setCurrentRow(ui.imageListWidget->count()-1);
      //    ui.setIsosurfaceButton->setEnabled(true);
      //    ui.setImageMaskButton->setEnabled(true);
      ui.setImageDataButton->setEnabled(true);
      //    mExportAction->setEnabled(true);
    }
  else
    {
      this->output(QString("Error loading ") + fname);
      int ret = QMessageBox::warning(this, tr("WSE"),
				     tr("There was an error loading image at $1!\n").arg(fname) + errStr,
				     QMessageBox::Ok);
      return ret;
      
    }
  QFileInfo fi(fname);
  QString path = fi.canonicalPath();
  if (!path.isNull()) {  g_settings->setValue("import_path", path); }
  
  this->syncRegisteredImageComboBoxes();
  return true;
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


void wseGUI::colorSchemeSelectorChanged(int val)
{
  //  mIsoRenderer->setBackgroundColor(mColorSchemes[val].background.r,
  //                               mColorSchemes[val].background.g,
  //                               mColorSchemes[val].background.b);

  //redrawIsoSurface();
}

void wseGUI::updateProgress(int p) 
{
  mProgressBar->setValue(p);
}

void wseGUI::updateHistogram() 
{
  this->output("Updating the histogram ...");
  
  // If no image data is set, do not update histogram
  if (mImageData < 0) 
    {
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

  FloatImage::itkImageType::Pointer image = mImageStack->image(mImageData)->itkImage();
  FloatImage::itkImageType::Pointer mask(NULL);

  if (mImageMask >= 0) {
    mask = mImageStack->image(mImageMask)->itkImage();
    mHistogram = new Histogram<FloatImage::itkImageType>(image, mask, numBins);
  } else {
    mHistogram = new Histogram<FloatImage::itkImageType>(image, numBins);
  }
    
  //  ui.lowerThresholdSpinBox->setRange(mHistogram->min(), mHistogram->max());
  //  ui.upperThresholdSpinBox->setRange(mHistogram->min(), mHistogram->max());

  this->output(QString("histogram min, max = %1, %2").arg(mHistogram->min()).arg(mHistogram->max()));
  this->output(QString("histogram mean, stdev = %1, %2").arg(mHistogram->mean()).arg(mHistogram->stdev()));
  this->output(QString("histogram pixel count = %1").arg(mHistogram->pixelCount()));

  mPoints.clear();
  for (unsigned int bin = 0; bin < mHistogram->numBins(); bin++) {
    mPoints.push_back(mHistogram->frequencies()[bin]);
  }
 
  updateHistogramBars();
  updateHistogramWidget();
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

  //  for (unsigned int i = 0; i < mMarkers.size(); i++) {
  //    markers.push_back(mMarkers[i]);
  //  }
  
  ui.histogramSlider_1->setHistogram(points, markers, mHistogram->min(), mHistogram->max());
  ui.histogramSlider_1->update();
}


void wseGUI::thresholdChanged(double lower, double upper) 
{
  
  if (mHistogram == NULL) 
    {
      return;
    }
  float min = mHistogram->min();
  float max = mHistogram->max();
  
  mThresholdLower = (max-min) * lower + min;
  mThresholdUpper = (max-min) * upper + min;
  
  mSliceViewer->SetThreshold(mThresholdLower, mThresholdUpper);
  mSliceViewer->Render();
  //  mIsoRenderer->setThreshold(mThresholdLower, mThresholdUpper);
  
  //  mThresholdTimer.stop();
  //  mThresholdTimer.start(100);
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

//void wseGUI::thresholdTimerEvent() {
  // mIsoRenderer->updateIsoScalars();
//  updateColorMap();
  // redrawIsoSurface();
//  reportThreshold();
//}


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




void wseGUI::toggleFullScreen() 
{
  if (mFullScreen) 
    {
      showNormal();
    } else 
    {
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
  if (index == 0) 
    {
      // mIsoRenderer->setScalarOnWall(true);
      if (ui.wallSurfaceComboBox->currentIndex() == ISO_NOT_SHOWN) 
	{
	  ui.wallSurfaceComboBox->setCurrentIndex(ISO_SURFACE);
	}
    } 
  else 
    {
      // mIsoRenderer->setScalarOnWall(false);
      ui.wallSurfaceComboBox->setCurrentIndex(ISO_NOT_SHOWN);
      if (ui.endoSurfaceComboBox->currentIndex() == ISO_NOT_SHOWN) 
	{
	  ui.endoSurfaceComboBox->setCurrentIndex(ISO_SURFACE);
	}
    }
  //  visualizePage();
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

// void wseGUI::setThresholdToIntensity( double value ) 
// {
//   float min = mHistogram->min();
//   float max = mHistogram->max();

//   float lowerThreshold = (value - min) / (max-min);
//   float upperThreshold = 100.0f;
  
//   ui.histogramSlider_1->setLowerThreshold(lowerThreshold);
//   ui.histogramSlider_1->setUpperThreshold(upperThreshold);
//   thresholdChanged(ui.histogramSlider_1->getLowerThreshold(), ui.histogramSlider_1->getUpperThreshold());
//   reportThreshold();
// }

// void wseGUI::setThresholdToIntensity( double lower, double upper )
// {
//   if (lower == mThresholdLower && upper == mThresholdUpper) 
//     { return;  }
  
//   float min = mHistogram->min();
//   float max = mHistogram->max();

//   float lowerThreshold = (lower - min) / (max-min);
//   float upperThreshold = (upper - min) / (max-min);

//   ui.histogramSlider_1->setLowerThreshold(lowerThreshold);
//   ui.histogramSlider_1->setUpperThreshold(upperThreshold);
//   thresholdChanged(ui.histogramSlider_1->getLowerThreshold(), ui.histogramSlider_1->getUpperThreshold());
//   reportThreshold();
// }


void wseGUI::showStatusMessage( const QString &text, int timeout /*= 0*/ ) 
{
  //  std::cerr << text.toStdString() << std::endl;
  ui.statusBar->showMessage(text, timeout);
}

void wseGUI::colorMapChanged( int m ) 
{
  mCurrentColorMap = m;
  updateColorMap();
}

void wseGUI::updateColorMap() 
{
  
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

void wseGUI::on_histogramBarsComboBox_currentIndexChanged( int index ) 
{
  updateHistogramBars();
  updateHistogramWidget();
  updateColorMap();
}

void wseGUI::on_histogramColorScaleCheckBox_stateChanged( int ) 
{
  updateColorMap();
}

void wseGUI::on_snapColorsToBarsCheckBox_stateChanged( int ) 
{
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

  ui.vtkSegmentationWidget->GetRenderWindow()->GetInteractor()->RemoveObservers(vtkCommand::MouseWheelBackwardEvent);
  ui.vtkSegmentationWidget->GetRenderWindow()->GetInteractor()->RemoveObservers(vtkCommand::MouseWheelForwardEvent);
  ui.vtkSegmentationWidget->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::MouseWheelBackwardEvent, mVTKCallback);
  ui.vtkSegmentationWidget->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::MouseWheelForwardEvent, mVTKCallback);

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
  for (int i = 0; i < 5 ;i++) 
    {
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


void wseGUI::on_executeDenoisingButton_accepted()
{
  // No image data selected.
  // TODO: Add message boxes for errors
  if (ui.denoisingInputComboBox->currentIndex() == -1) 
    {
      QMessageBox::warning(this, "WSE", QString("Please select image data first."));
      return;
    }
  
  // Are we processing?
  if (mITKFilteringThread->isFiltering() || mITKSegmentationThread->isFiltering())
    {
      QMessageBox::warning(this, "WSE", QString("Please wait until the current filtering operation has finished."));
      return;
    }
  
  if (ui.gaussianRadioButton->isChecked())         { this->runGaussianFiltering();    }
  else if (ui.anisotropicRadioButton->isChecked()) { this->runAnisotropicFiltering(); }
  else if (ui.curvatureRadioButton->isChecked())   { this->runCurvatureFiltering();   }
}

void wseGUI::on_executeGradientButton_accepted()
{
  // No image data selected.
  // TODO: Add message boxes for errors
  if (ui.gradientInputComboBox->currentIndex() == -1) 
    {
      QMessageBox::warning(this, "WSE", QString("Please select image data first."));
      return;
    }
  
  // Are we processing?
  if (mITKFilteringThread->isFiltering() || mITKSegmentationThread->isFiltering())
    {
      QMessageBox::warning(this, "WSE", QString("Please wait until the current filtering operation has finished."));
      return;
    }
  
  this->runGradientFiltering();
}

void wseGUI::on_executeWatershedsButton_accepted()
{
  // No image data selected.
  // TODO: Add message boxes for errors
  if (ui.watershedInputComboBox->currentIndex() == -1) 
    {
      QMessageBox::warning(this, "WSE", QString("Please select image data first."));
      return;
    }
  
  // Are we processing?
  if (mITKFilteringThread->isFiltering() || mITKSegmentationThread->isFiltering())
    {
      QMessageBox::warning(this, "WSE", QString("Please wait until the current filtering operation has finished."));
      return;
    }
  
  this->runWatershedSegmentation();

}


void wseGUI::floodLevelChanged()
{
  // Get flood level from the interface
  float lvl = static_cast<float>(ui.floodLevelA->value()) 
    + static_cast<float>(ui.floodLevelB->value()) * 0.01;

  // Update flood level display
  ui.floodLevelLabel->setText(QString("%1").arg(lvl,0,'g',4));

  // Set the segmentation 
  //  mSegmentSliceViewermanager->ClearHighlightedValuesToSameColor();
  mSegmentation->Merge(lvl / 100.0);
  this->updateImageDisplay();
}

} // end namespace
