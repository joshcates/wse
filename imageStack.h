#ifndef IMAGESTACK_H
#define IMAGESTACK_H

#include <QtGui>
#include "image.h"

/** */
class imageStack : public QObject
{
Q_OBJECT

public:
  imageStack();
  ~imageStack();

  // Images
  int numImages() { return mImages.size(); }
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
  vtkImageImport *selectedImageVTK(bool original);
  const vtkImageImport *selectedImageVTK(bool original) const;

  // Processing
  //  void bilateralFilter(int kernel, double colorSigma, double spaceSigma);
  void resetFilters();

signals:
  void progressChanged(int p);

private:
  /** Convert an ITK 2D image to a QImage */
  //  QImage ITKImageToQImage(Image::itkFloatImage::Pointer img);

  int mSelectedImage;
  std::vector<Image *> mImages;
};

#endif // IMAGESTACK_H
