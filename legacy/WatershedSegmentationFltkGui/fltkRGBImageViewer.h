/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: fltkRGBImageViewer.h,v $
  Language:  C++
  Date:      $Date: 2003-05-01 23:18:26 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __fltkRGBImageViewer_h
#define __fltkRGBImageViewer_h

#include "fltkRGBImageViewerGUI.h"

namespace fltk {

template <class ImagePixelType, class OverlayPixelType>
class RGBImageViewer : public fltkRGBImageViewerGUI
{
public:

   /**
   * Standard "Self" typedef.
   */
  typedef RGBImageViewer         Self;

  /** 
   * Smart pointer typedef support.
   */
  typedef SmartPointer<Self>        Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

 
  typedef itk::Image< itk::RGBPixel<ImagePixelType>, 3 >   ImageType;
  typedef itk::Image< OverlayPixelType, 3 > OverlayType;
  typedef GLColorSliceView< ImagePixelType, OverlayPixelType > 
                                                   GLColorSliceViewType;
  typedef typename GLColorSliceViewType::ColorTablePointer ColorTablePointer;


    
  RGBImageViewer();
  virtual ~RGBImageViewer();
  virtual void SetImage(ImageBase<3> * img);
  virtual void SetOverlay(ImageBase<3> * img);
  virtual void Show(void);
  virtual void Hide(void);
  virtual void Update(void);
  virtual void Synchronize(void);
  virtual void ImageMode(ImageModeType mode);
  virtual void SelectSlice(unsigned int num);
  virtual void SetIntensityWindowingMin(float val);
  virtual void SetIntensityWindowingMax(float val);
  virtual void CenterWindow(void);
  virtual void ZoomIn(void);
  virtual void ZoomOut(void);
  virtual void ShiftUp(void);
  virtual void ShiftDown(void);
  virtual void ShiftLeft(void);
  virtual void ShiftRight(void);
  virtual void SetOrientation(void);
  virtual void CreateGLColorSliceView( Fl_Group * g , Fl_Gl_Window * w );
  virtual void SetLabel( const char * );
  virtual void ShowClickedPointsWindow(void);
  virtual void UpdateClickedPoints(void);
  virtual void ClearClickedPoints(void);
  virtual void ClickSelectCallBack(
                void (*newClickSelectArgCallBack)(float, float,
                                                  float, float,
                                                  void *),
                     void * newClickSelectArg);

  
  virtual void ViewDetails(bool detail);
  virtual void ViewValue(bool value);
  virtual void ViewCrosshairs(bool crosshairs);

  /** Specify the opacity of the overlay */
  virtual void  SetOverlayOpacity(float newOverlayOpacity);
  
  /** Get the opacity of the overlay */
  virtual float GetOverlayOpacity(void) const;
  
  /** Show slider to control overlay opacity */
  virtual void ShowOverlayOpacityControl(void);
  
  /** Get the ColorTable for the Overlay */
  virtual ColorTablePointer GetOverlayColorTable(void);

  /** Set the ColorTable for the Overlay */
  virtual void SetOverlayColorTable(ColorTablePointer newColorTable);


private:
  GLColorSliceViewType * glSliceView;
};


} // end namespace fltk

#ifndef ITK_MANUAL_INSTANTIATION
#include <fltkRGBImageViewer.txx>
#endif




#endif
