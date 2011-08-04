#ifndef WSE_ISORENDERER_H
#define WSE_ISORENDERER_H

#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
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
#include "vtkImageViewer2.h"
#include "vtkPolyDataNormals.h"
#include "vtkMarchingCubes.h"
#include "vtkImageMapToColors.h"
#include "vtkImageBlend.h"

#include "image.h"
//#include "scalarMethod.h"

class IsoRenderer {

  // -- constructor/destructor --
public:
  IsoRenderer();
  virtual ~IsoRenderer();


  // -- public methods --
public:

  void setImageData(Image *image);
  void setWallData(Image *image);
  void setEndoData(Image *image);
  void setRenderMethods(std::string wallRenderType, std::string endoRenderType);
  void setSlice(vtkAlgorithmOutput *slice);
  void setClipDirection(bool direction);
  void setShowNormals(bool showNormals);
  void setClipSurfaceToSlice(bool clipSurface);
  void setSliceVisibility(bool sliceVisible);
  void setBackgroundColor(float r, float g, float b);
  void setLinearInterpolation(bool image, bool mask);
  void setScalarMethod(ScalarMethod method);
  void setSliceDisplayExtent(int *extent);
  //  void setSmoothSettings(SmoothSettings settings);
  void setThreshold(float lower, float upper);
  void setSmoothThresholdWidth(float value);
  void setScalarOnWall(bool scalarOnWall);
  void setSmoothThreshold(bool value);
  void setMeshSubdivision(bool value);

  void createIsoSurfaces();
  void redrawIsoSurface();
  void updateClipPlane();
  void updateHedgeHogs();
  void updateIsoScalars();
  void updateSlice();

  vtkRenderer *getSurfaceRenderer();
  vtkLookupTable *getLookupTable();
  float getScalarMin();
  float getScalarMax();

  // -- private methods and members --
private:

  void installPipeline();
  void updateRenderType();
  void updateClipToSurface();
  void updateSliceVisibility();
  void updateBackgroundColor();

  void setIsoScalars(vtkPolyDataNormals *polyDataNormals, bool negativeNormal);
  void eraseIsoScalars(vtkPolyDataNormals *polyDataNormals);
  float computeIsoScalar(double *point, double *normal, bool negativeNormal);
  float getInterpolatedValue( Image *image, double p[3], bool linear);

  float applyThreshold(float value);

  Image *mImageData;
  Image *mWallData;
  Image *mEndoData;
  float mScalarMax;
  float mScalarMin;

  // -- vtk objects --
  vtkRenderer *mSurfaceRenderer;
  vtkContourFilter *mEndoContourFilter;
  vtkMarchingCubes *mEndoMarchingCubes;
  vtkPolyDataMapper *mEndoContourMapper;
  vtkPolyDataNormals *mEndoPolyDataNormals;
  vtkActor *mEndoSurfaceActor;
  vtkContourFilter *mWallContourFilter;
  vtkMarchingCubes *mWallMarchingCubes;
  vtkPolyDataMapper *mWallContourMapper;
  vtkPolyDataNormals *mWallPolyDataNormals;
  vtkActor *mWallSurfaceActor;
  vtkPolyDataMapper *mHedgeHogMapper;
  vtkPolyDataMapper *mVTKPolyDataMapper;
  vtkLookupTable *mSliceStencilLUT;
  vtkImageMapToColors *mSliceStencilMap;
  vtkImageBlend *mSliceStencilBlend;
  vtkActor *mHedgeHogActor;
  vtkImageActor *mSliceActor;
  vtkPlane *mClipPlane;
  vtkLookupTable *mScalarLUT;
  vtkAlgorithmOutput *mSlice;


  // -- render state --
  std::string mWallRenderType;
  std::string mEndoRenderType;
  bool mClipDirection;
  bool mShowNormals;
  bool mClipSurfaceToSlice;
  bool mSliceVisible;
  float mBackgroundColorRed;
  float mBackgroundColorGreen;
  float mBackgroundColorBlue;
  bool mScalarOnWall;
  bool mCameraSet;
  bool mDataLinearInterpolation;
  bool mMaskLinearInterpolation;
  ScalarMethod mScalarMethod;
  //  SmoothSettings mSmoothSettings;
  bool mSmoothThreshold;
  bool mSubDivideMesh;
  float mThresholdLower;
  float mThresholdUpper;
  float mSmoothThresholdWidth;


public:
  // -- constants --
  const static std::string RENDER_SURFACE;
  const static std::string RENDER_MESH;
  const static std::string RENDER_POINTS;
  const static std::string RENDER_HIDDEN;

};


#endif
