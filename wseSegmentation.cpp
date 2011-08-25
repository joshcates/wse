#include "wseSegmentation.h"

namespace wse {

Segmentation::Segmentation(ULongImage *img, SegmentTreeType *tree)
{
  mWatershedTransform = img;
  mSegmentTree        = tree;

  // Allocate the lookup table manager object
  mLUTManager = vtkWSLookupTableManager::New();
  mLUTManager->SetHighlightColor(1.0, 1.0, 1.0);
  mLUTManager->SetRepaintHighlights(1);
  mLUTManager->Initialize();
  mLUTManager->LoadTree(tree);
  mLUTManager->SetNumberOfLabels(mWatershedTransform->computeMaximumImageValue());
  mLUTManager->GenerateColorTable();
  
  // Set up the color mapping
  // mImageMapToRGBA = vtkImageMapToColors::New();
  // mImageMapToRGBA->SetOutputFormatToRGBA();
  // mImageMapToRGBA->SetInputConnection(img->vtkImporter()->GetOutputPort());
  // mImageMapToRGBA->SetLookupTable(mLUTManager->GetLookupTable());

}

Segmentation::~Segmentation() 
{

  // TODO:  Delete VTK objects here

}


}
