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
  mLUTManager->SetNumberOfLabels(mWatershedTransform->computeMaximumImageValue()+1);
  mLUTManager->PassAlphaToOutputOff();
  mLUTManager->GenerateColorTable();

}

Segmentation::~Segmentation() 
{

  // TODO:  Delete VTK objects here

}


}
