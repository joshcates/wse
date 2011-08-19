#ifndef _wse_imageStack_hxx
#define _wse_imageStack_hxx

#include <QtGui>
#include "wseImage.hxx"

namespace wse {

/** This is a convenience class for managing a stack of wse::Image
    pointers. It includes the concept of a "currently selected" image.
    Useful for underneath GUIs. */
template <class T>
class imageStack : public QObject
{

public:
  imageStack() : mSelectedImage(-1) {}
  ~imageStack();

  /** Returns the number of images in the stack. */
  int numImages() const { return mImages.size(); }

  /** Adds a wse::Image pointer to the stack. */
  bool addImage(Image<T> *img) 
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
  Image<T> *image(unsigned int i);
  const Image<T> *image(unsigned int i) const;

  /** Returns the wse::Image pointer of the "selected image".  Returns
      a NULL pointer if no image is selected.*/
  Image<T> *selectedImage();
  const Image<T> *selectedImage() const;

  /** Returns the image color at location "i" in the stack. */
  QColor imageColor(unsigned int i) const;

  /** Returns the image name at location "i" in the stack. */
  QString name(unsigned int i) const;

  /** Returns the name of the currently selected image. */
  QString selectedName() const;

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
  std::vector<Image<T> *> mImages;
};

template<class T>
imageStack<T>::~imageStack()
{
  // TODO: Clean up images stored in this stack
  for (unsigned int i = 0; i < mImages.size(); i++)
    {
      //      delete mImages[i];
    }
}

template<class T>
bool imageStack<T>::addImage(QString fname)
{
  Image<T> *img = new Image<T>();
  if (!img->read(fname))  return false;
  mImages.push_back(img);

  mSelectedImage = mImages.size()-1;
  return true;
}

template<class T>
bool imageStack<T>::removeImage(QString fname)
{
  for (unsigned int i = 0; i < mImages.size(); i++)
  {
    if (fname.compare(mImages[i]->name())==0)
    {
      delete mImages[i];
      mImages.erase(mImages.begin()+i);
      if (mSelectedImage == (int) i) { mSelectedImage = -1; }
      return true;
    }
  }

  return false;
}

template<class T>
Image<T> *imageStack<T>::selectedImage()
{
  if (mSelectedImage >= 0)
    {    return mImages[mSelectedImage];  }
  else
    return NULL;
}

template<class T>
const Image<T> *imageStack<T>::selectedImage() const
{
  if (mSelectedImage >= 0)
    {  return mImages[mSelectedImage];   }
  else
    return NULL;
}

template<class T>
Image<T> *imageStack<T>::image(unsigned int i)
{
  if (i >= 0 && i < mImages.size())
    {    return mImages[i];    }
  else return NULL;
}

template<class T>
const Image<T> *imageStack<T>::image(unsigned int i) const
{
  if (i >= 0 && i < mImages.size())
    {    return mImages[i];    }
  else return NULL;
}

template<class T>
QColor imageStack<T>::imageColor(unsigned int i) const
{
  if (i >= 0 && mImages.size() > i)
    return mImages[i]->color();
  else
    return QColor(0,0,0);
}

template<class T>
QString imageStack<T>::name(unsigned int i) const
{
  if (i >=0 && i < mImages.size())
  {    return mImages[i]->name();  }
  return QString();
}

template<class T>
QString imageStack<T>::selectedName() const
{
  if (mSelectedImage >= 0)
  {    return mImages[mSelectedImage]->name();  }
  return QString();
}

template<class T>
bool imageStack<T>::setSelectedByName(QString fname)
{
  bool found = false;

  for (unsigned int i = 0; i < mImages.size(); i++)
  {
    if (fname.compare(mImages[i]->name())==0)
      {
	mSelectedImage = i;
	found = true;
      }
  }
  return found;
}

// Some common stack types
typedef imageStack<float> FloatImageStack;
typedef imageStack<unsigned long> ULongImageStack;

} // end namespace wse

#endif
