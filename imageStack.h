#ifndef _wse_imageStack_h_
#define _wse_imageStack_h_

#include <QtGui>
#include "image.h"

namespace wse {

/** NEEDS DOCUMENTATION */
class imageStack : public QObject
{
Q_OBJECT

public:
  imageStack() : mSelectedImage(-1) {}
  ~imageStack();

  // Images
  int numImages() { return mImages.size(); }
  bool addImage(Image *img) 
  {
    mImages.push_back(img); 
    mSelectedImage = mImages.size()-1;
    return true;
  }

  bool addImage(QString fname);
  bool removeImage(QString fname);
  Image *image(unsigned int i);
  const Image *image(unsigned int i) const;
  QColor imageColor(unsigned int i);
  QString name(unsigned int i);

  // Selection
  QString selectedName();
  bool setSelectedByName(QString fname);
  void clearSelection();
  Image *selectedImage();
  const Image *selectedImage() const;

  /** Returns the vtkImageImport connected to the selected image. */
  vtkImageImport *selectedImageVTK()
  {   return mImages[mSelectedImage]->vtkImporter();  }
  const vtkImageImport *selectedImageVTK() const
  {   return mImages[mSelectedImage]->vtkImporter();  }

  // Processing
  void resetFilters();

signals:
  void progressChanged(int p);

private:
  /** Convert an ITK 2D image to a QImage */
  //  QImage ITKImageToQImage(Image::itkFloatImage::Pointer img);

  int mSelectedImage;
  std::vector<Image *> mImages;
};

} // end namespace wse

#endif // IMAGESTACK_H
