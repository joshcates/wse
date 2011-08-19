#ifndef _wse_imageStack_h_
#define _wse_imageStack_h_

#include <QtGui>
#include "image.h"

namespace wse {

/** This is a convenience class for managing a stack of wse::Image
    pointers. It includes the concept of a "currently selected" image.
    Useful for underneath GUIs. */
class imageStack : public QObject
{

public:
  imageStack() : mSelectedImage(-1) {}
  ~imageStack();

  /** Returns the number of images in the stack. */
  int numImages() const { return mImages.size(); }

  /** Adds a wse::Image pointer to the stack. */
  bool addImage(Image *img) 
  {
    mImages.push_back(img); 
    mSelectedImage = mImages.size()-1;
    return true;
  }

  /** Loads the image from "fname" and adds it as an wse:Image to the stack. */
  bool addImage(QString fname);

  /** Removes the image with name "fname" from the stack. */
  bool removeImage(QString fname);

  /** Returns the wse::Image pointer at location "i" in the stack. Returns
      NULL pointer if no image exists. */
  Image *image(unsigned int i);
  const Image *image(unsigned int i) const;

  /** Returns the wse::Image pointer of the "selected image".  Returns
      a NULL pointer if no image is selected.*/
  Image *selectedImage();
  const Image *selectedImage() const;

  /** Returns the image color at location "i" in the stack. */
  QColor imageColor(unsigned int i);

  /** Returns the image name at location "i" in the stack. */
  QString name(unsigned int i);

  /** Returns the name of the currently selected image. */
  QString selectedName();

  /** Sets the "selected image" to the image that matches "fname".
      Returns true if the image name was found and false otherwise. */
  bool setSelectedByName(QString fname);

  /** Sets the "selected image" index to a null value of -1. */
  void clearSelection()  { mSelectedImage = -1; }
  
    /** Returns the vtkImageImport connected to the selected image. */
  vtkImageImport *selectedImageVTK()
  {   return mImages[mSelectedImage]->vtkImporter();  }
  const vtkImageImport *selectedImageVTK() const
  {   return mImages[mSelectedImage]->vtkImporter();  }

private:
  /** Convert an ITK 2D image to a QImage */
  //  QImage ITKImageToQImage(Image::itkFloatImage::Pointer img);

  int mSelectedImage;
  std::vector<Image *> mImages;
};

} // end namespace wse

#endif // IMAGESTACK_H
