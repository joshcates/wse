#ifndef _wse_slice_viewer_h
#define _wse_slice_viewer_h

#include "vtkObject.h"

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkImageData.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkInteractorStyleImage.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkImageMapper.h"
#include "vtkLookupTable.h"
#include "vtkAlgorithmOutput.h"
#include "vtkImageBlend.h"
#include "vtkImageThreshold.h"
#include "vtkImageStencil.h"
#include "vtkImageToImageStencil.h"
#include "vtkImageMask.h"
#include "vtkImageFlip.h"
#include "vtkPointPicker.h"

namespace wse {

/** SliceViewer is a class based on the vtkImageViewer class.  It
    provides similar functionality, but is tailored for working with
    3D floating point grayscale image displays (e.g. medical image
    volumes).

    The following VTK imageViewer documentation also applies to
    SliceViewer:

    "ImageViewer is a convenience class for displaying a 2D image.  It
    packages up the functionality found in vtkRenderWindow,
    vtkRenderer, vtkImageActor and vtkImageMapToWindowLevelColors into
    a single easy to use class.  This class also creates an image
    interactor style (vtkInteractorStyleImage) that allows zooming and
    panning of images, and supports interactive window/level
    operations on the image. Note that ImageViewer is simply a wrapper
    around these classes.

    ImageViewer uses the 3D rendering and texture mapping engine to
    draw an image on a plane.  This allows for rapid rendering, zooming,
    and panning. The image is placed in the 3D scene at a depth based on
    the z-coordinate of the particular image slice. Each call to
    SetSlice() changes the image data (slice) displayed AND changes the
    depth of the displayed slice in the 3D scene. This can be controlled
    by the AutoAdjustCameraClippingRange ivar of the InteractorStyle
    member."
 */
class SliceViewer : public vtkObject
{
public:
  static SliceViewer *New();
  vtkTypeRevisionMacro(SliceViewer,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /** Get the name of rendering window.*/
  virtual const char *GetWindowName();

  /** Render the resulting image. */
  virtual void Render(void);

  /** Set/Get the input image to the viewer. */
  virtual void SetInput(vtkImageData *in);
  virtual vtkImageData *GetInput();
  virtual void SetInputConnection(vtkAlgorithmOutput* input);

  /** Disassemble the rendering pipeline. */
  virtual void DisableDisplay();

  /** set transparent image mask */
  virtual void SetImageMask(vtkAlgorithmOutput* mask);

  /** Set/get the slice orientation */
  //BTX
  enum
  {
    SLICE_ORIENTATION_YZ = 0,
    SLICE_ORIENTATION_XZ = 1,
    SLICE_ORIENTATION_XY = 2
  };
  //ETX
  vtkGetMacro(SliceOrientation, int);
  virtual void SetSliceOrientation(int orientation);
  virtual void SetSliceOrientationToXY()
    { this->SetSliceOrientation(SliceViewer::SLICE_ORIENTATION_XY); };
  virtual void SetSliceOrientationToYZ()
    { this->SetSliceOrientation(SliceViewer::SLICE_ORIENTATION_YZ); };
  virtual void SetSliceOrientationToXZ()
    { this->SetSliceOrientation(SliceViewer::SLICE_ORIENTATION_XZ); };

  // Description:
  // Set/Get the current slice to display (depending on the orientation
  // this can be in X, Y or Z).
  vtkGetMacro(Slice, int);
  virtual void SetSlice(int s);

  // Description:
  // Update the display extent manually so that the proper slice for the
  // given orientation is displayed. It will also try to set a
  // reasonable camera clipping range.
  // This method is called automatically when the Input is changed, but
  // most of the time the input of this class is likely to remain the same,
  // i.e. connected to the output of a filter, or an image reader. When the
  // input of this filter or reader itself is changed, an error message might
  // be displayed since the current display extent is probably outside
  // the new whole extent. Calling this method will ensure that the display
  // extent is reset properly.
  virtual void UpdateDisplayExtent();

  // Description:
  // Return the minimum and maximum slice values (depending on the orientation
  // this can be in X, Y or Z).
  virtual int GetSliceMin();
  virtual int GetSliceMax();
  virtual void GetSliceRange(int range[2])
    { this->GetSliceRange(range[0], range[1]); }
  virtual void GetSliceRange(int &min, int &max);
  virtual int* GetSliceRange();

  // Description:
  // Set window and level for mapping pixels to colors.
  virtual double GetColorWindow();
  virtual double GetColorLevel();
  virtual void SetColorWindow(double s);
  virtual void SetColorLevel(double s);

  // Description:
  // These are here when using a Tk window.
  virtual void SetDisplayId(void *a);
  virtual void SetWindowId(void *a);
  virtual void SetParentId(void *a);

  // Description:
  // Set/Get the position in screen coordinates of the rendering window.
  virtual int* GetPosition();
  virtual void SetPosition(int a,int b);
  virtual void SetPosition(int a[2]) { this->SetPosition(a[0],a[1]); }

  // Description:
  // Set/Get the size of the window in screen coordinates in pixels.
  virtual int* GetSize();
  virtual void SetSize(int a, int b);
  virtual void SetSize(int a[2]) { this->SetSize(a[0],a[1]); }

  // Description:
  // Get the internal render window, renderer, image actor, and
  // image map instances.
  vtkGetObjectMacro(RenderWindow,vtkRenderWindow);
  vtkGetObjectMacro(Renderer, vtkRenderer);
  vtkGetObjectMacro(ImageActor,vtkImageActor);
  vtkGetObjectMacro(WindowLevel,vtkImageMapToWindowLevelColors);
  vtkGetObjectMacro(InteractorStyle,vtkInteractorStyleImage);

  // Description:
  // Set your own renderwindow and renderer
  virtual void SetRenderWindow(vtkRenderWindow *arg);
  virtual void SetRenderer(vtkRenderer *arg);

  // Description:
  // Attach an interactor for the internal render window.
  virtual void SetupInteractor(vtkRenderWindowInteractor*);

  // Description:
  // Create a window in memory instead of on the screen. This may not
  // be supported for every type of window and on some windows you may
  // need to invoke this prior to the first render.
  virtual void SetOffScreenRendering(int);
  virtual int GetOffScreenRendering();
  vtkBooleanMacro(OffScreenRendering,int);

  // Description:
  // @deprecated Replaced by sliceviewer::GetSliceMin() as of VTK 5.0.
  VTK_LEGACY(int GetWholeZMin());

  // Description:
  // @deprecated Replaced by sliceviewer::GetSliceMax() as of VTK 5.0.
  VTK_LEGACY(int GetWholeZMax());

  // Description:
  // @deprecated Replaced by sliceviewer::GetSlice() as of VTK 5.0.
  VTK_LEGACY(int GetZSlice());

  // Description:
  // @deprecated Replaced by sliceviewer::SetSlice() as of VTK 5.0.
  //VTK_LEGACY(void SetZSlice(int));
  void SetZSlice(int);

  /** */
  void SetImageLookupTable(vtkLookupTable *l);

  void SetShowMask(bool);
  void SetShowThreshold(bool);
  void SetMaskOpacity(float);
  void SetThresholdOpacity(float);
  void SetThreshold(float, float);
  void SetClipThresholdToMask(bool);

  vtkAlgorithmOutput* GetFinalOutput();

  vtkRenderWindowInteractor *GetInteractor()
  {return Interactor; }
  
protected:
  SliceViewer();
  ~SliceViewer();

  virtual void InstallPipeline();
  virtual void UnInstallPipeline();

  vtkImageMapToWindowLevelColors  *WindowLevel;
  vtkImageMapToWindowLevelColors  *MaskWindowLevel;
  vtkImageMapToColors             *MaskImageMapToColors;
  vtkRenderWindow                 *RenderWindow;
  vtkRenderer                     *Renderer;
  vtkImageActor                   *ImageActor;
  vtkImageActor                   *MaskImageActor;
  vtkRenderWindowInteractor       *Interactor;
  vtkInteractorStyleImage         *InteractorStyle;
  vtkImageBlend                   *mImageBlend;
  //  vtkImageFlip                    *mImageFlip;

  vtkLookupTable                  *mImageLookupTable;

  vtkAlgorithmOutput              *mImage;
  vtkAlgorithmOutput              *mMask;
  vtkAlgorithmOutput              *mFinalOutput;

  vtkLookupTable *mMaskLUT;

  bool mPipelineInstalled;

  int SliceOrientation;
  int FirstRender;
  int Slice;

  bool mShowMask;
  bool mShowThreshold;
  bool mClipThresholdToMask;

  vtkImageThreshold *mImageThreshold;
  float mThresholdLower;
  float mThresholdUpper;
  float mMaskOpacity;
  float mThresholdOpacity;

  vtkImageBlend *mFinalBlend;
  vtkImageBlend *mStencilBlend;
  vtkLookupTable *mAlphaLUT;
  vtkLookupTable *mStencilLUT;
  vtkLookupTable *mThresholdLUT;
  vtkImageMapToColors *mAlphaMap;
  vtkImageMapToColors *mThresholdImageMapToColors;
  vtkImageMapToColors *mStencilMap;

  virtual void UpdateOrientation();

private:
  SliceViewer(const SliceViewer&);  // Not implemented.
  void operator=(const SliceViewer&);  // Not implemented.

  /** */
  virtual void ResetWindowLevel(vtkImageMapToWindowLevelColors *windowLevel);

  /** */
  virtual void UpdateDisplay();
};


class imageViewerCallback : public vtkCommand
{
public:
  static imageViewerCallback *New() { return new imageViewerCallback; }

  void Execute(vtkObject *caller, unsigned long event, void *vtkNotUsed(callData));

  SliceViewer *IV;
  double InitialWindow;
  double InitialLevel;
};




} // end namespace wse
#endif // _wse_slice_viewer_h
