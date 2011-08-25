#include "wseSegmentation.h"

namespace wse {

Segmentation::Segmentation(ULongImage::itkImageType *img, SegmentTreeType *tree)
{
  mWatershedTransform = img;
  mSegmentTree        = tree;

  // Allocate the lookup table manager object
  mLUTManager = vtkWSLookupTableManager::New();
  mLUTManager->SetHighlightColor(1.0, 1.0, 1.0);
  mLUTManager->SetRepaintHighlights(1);
  mLUTManager->LoadTree(tree);

  

}

Segmentation::~Segmentation() 
{

}


}
