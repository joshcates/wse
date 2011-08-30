#ifndef _segmentation_viewer_h_
#define _segmentation_viewer_h_

#include "wseSliceViewer.h"

namespace wse {

/** This is a subclass of SliceViewer that is designed for displaying
    labeled image maps.  It redefines certain functions
    (e.g. window/leveling) to be more appropriate for labeled images.
    See wse::SliceViewer for more information. */
class SegmentationViewer : public SliceViewer
{
public:
  static SegmentationViewer *New();
  vtkTypeRevisionMacro(SegmentationViewer,SliceViewer);
  void PrintSelf(ostream& os, vtkIndent indent);

  /** Render the resulting image. */
  //  virtual void Render(void);

  // Description:
  // Attach an interactor for the internal render window.
  //virtual void SetupInteractor(vtkRenderWindowInteractor*);


protected:
  SegmentationViewer();
  ~SegmentationViewer();

  virtual void InstallPipeline();

private:
  SegmentationViewer(const SegmentationViewer&);  // Not implemented.
  void operator=(const SegmentationViewer&);  // Not implemented.

  virtual void ResetWindowLevel(vtkImageMapToWindowLevelColors *windowLevel);
};

} // end namespace wse


#endif // segmentationviewer_H
