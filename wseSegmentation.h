#ifndef _wseSegmentation_h_
#define _wseSegmentation_h_

// WSE Includes
#include "wseImage.hxx"

// ITK Includes
#include <itkImage.h>
#include <itkImageFileReader.h>

// Custom vtk / itk classes
#include "itkWatershedSegmentTreeWriter.h"
#include "itkScalarToRGBPixelFunctor.h"
#include "vtkWSLookupTableManager.h"
#include <vtkColorTransferFunction.h>
#include <vtkImageMapToColors.h>
#include "vtkWSBoundingBoxManager.h"
#include "vtkBinaryVolume.h"
#include "vtkBinaryVolumeLogic.h"

// Standard VTK classes
#include <vtkActor.h>
#include <vtkLookupTable.h>
#include <vtkImageMapper.h>
#include <vtkActor2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageThreshold.h>
#include <vtkContourFilter.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkImageCast.h>
#include <vtkImageMapToColors.h>

namespace wse {

/** */
class Segmentation
{
 public:
  // NOTE: ULongImage is defined in wseImage.hxx
  typedef itk::WatershedSegmentTreeWriter<float>::SegmentTreeType SegmentTreeType;

  /** Constructor takes a ULongImage pointer and a SegmentTreeType pointer*/
  Segmentation(ULongImage *, SegmentTreeType *t);
  ~Segmentation();

  /** Return the segment tree object. */
  SegmentTreeType::ConstPointer segmentTree() const
    { return SegmentTreeType::ConstPointer(mSegmentTree); }
  
  /** Return the wseImage of the watershed transform */
  const ULongImage *watershedTransform() const
  {    return mWatershedTransform;   }

  /** Return the itk::Image of the watershed transform */
  ULongImage::itkImageType::ConstPointer itkImage() const
    { return ULongImage::itkImageType::ConstPointer(mWatershedTransform->itkImage()); }

  /** Return the vtkImporter for the itk::Image of the watershed
      transform. */
  const vtkImageImport *vtkImporter() const
  { return mWatershedTransform->vtkImporter(); }
  vtkImageImport *vtkImporter()
  { return mWatershedTransform->vtkImporter(); }
 
  /** Returns the number of slices in the watershed transform image. */
  unsigned int nSlices() const
  { return mWatershedTransform->nSlices(); }
 
  /** Write the segmentation to disk.  ToDo: UNIMPLEMENTED */
  bool write() const { return false;}

  /** Read a segmentation from disk. ToDo: UNIMPLEMENTED */
  bool read(const char *fn) { return false; }

  /** Return an output port for the VTK rendering pipeline. */
  vtkAlgorithmOutput *GetOutputPort()
  { return mWatershedTransform->vtkImporter()->GetOutputPort(); }

  /** Returns the vtk color lookup table from the watershed manager. */
  vtkLookupTable *GetLookupTable()
  {
    return mLUTManager->GetLookupTable();
  }

  /** Merge the segments through the lookup table to the given level */
  void Merge(float l)
  {    mLUTManager->Merge(l);  }


 private:
  /** A wseImage wrapper around the labeled image of the watershed
      transform, which is one of the outputs of the
      itkWatershedImageFilter. */
  ULongImage* mWatershedTransform;

  /** The tree of all computed merges of the watershed catchment
      basins. This is the second output of the
      itkWatershedImageFilter. */
  SegmentTreeType::Pointer mSegmentTree;

  /** Logic for merging and splitting regions from the watershed transform. */
  vtkBinaryVolumeLogic* mBinaryLogic;

  /** Lookup table manager for the segmented image.  This object holds the segmentation merge tree. */
  vtkWSLookupTableManager* mLUTManager;

  /** Bounding box manager for the segmented image. */
  vtkWSBoundingBoxManager* mBoundingBoxManager;

};

} // end namespace wse

#endif // _wseSegmentation_h_x
