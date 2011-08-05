#include "imageStack.h"

imageStack::imageStack():
  mSelectedImage(-1)
{

}

imageStack::~imageStack()
{
  // TODO: Clean up images stored in this stack
  for (unsigned int i = 0; i < mImages.size(); i++)
    {
      //      delete mImages[i];
    }
}

bool imageStack::addImage(QString fname)
{
  Image *img = new Image();
  if (!img->load(fname))
    return false;
  mImages.push_back(img);

  mSelectedImage = mImages.size()-1;
  return true;
}

bool imageStack::removeImage(QString fname)
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


Image *imageStack::selectedImage()
{
  if (mSelectedImage >= 0)
    {
    return mImages[mSelectedImage];
    }
  else
    return NULL;
}

const Image *imageStack::selectedImage() const
{
  if (mSelectedImage >= 0)
    {
    return mImages[mSelectedImage];
    }
  else
    return NULL;
}

vtkImageImport *imageStack::selectedImageVTK(bool original)
{
  if (mSelectedImage >= 0)
  {
    //     if (original)
       return mImages[mSelectedImage]->originalVTK();
       //  else
       // return mImages[mSelectedImage]->modifiedVTK();
  }
  return 0;
}


const vtkImageImport *imageStack::selectedImageVTK(bool original) const
{
  if (mSelectedImage >= 0)
  {
    //     if (original)
       return mImages[mSelectedImage]->originalVTK();
       // else
       // return mImages[mSelectedImage]->modifiedVTK();
  }
  return 0;
}

//QImage ImageStack::ITKImageToQImage(Image::itkFloatImage::Pointer img)
//{
//
// const float * pixelBuffer = img->GetBufferPointer();
//  const int width  = img->GetBufferedRegion().GetSize()[0];
// const int height = img->GetBufferedRegion().GetSize()[1];
//  QRgb colorTable = NULL;
//  const int numColors = 256;
//  Endian bitOrder = IgnoreEndian;

//  return QImage(pixelBuffer,width,height,colorTable,numColors,bitOrder);
//}


Image *imageStack::image(unsigned int i)
{
  if (i >= 0 && i < mImages.size())
    {
    return mImages[i];
    }
  else return NULL;
}

const Image *imageStack::image(unsigned int i) const
{
  if (i >= 0 && i < mImages.size())
    {
    return mImages[i];

    }
  else return NULL;
}


QColor imageStack::imageColor(unsigned int i)
{
  if (i >= 0 && mImages.size() > i)
    return mImages[i]->color();
  else
    return QColor(0,0,0);
}


QString imageStack::name(unsigned int i)
{
  if (i >=0 && i < mImages.size())
  {
    return mImages[i]->name();
  }
  return QString();
}


QString imageStack::selectedName()
{
  if (mSelectedImage >= 0)
  {
    return mImages[mSelectedImage]->name();
  }
  return QString();
}


bool imageStack::setSelectedByName(QString fname)
{
  bool found = false;
  //  std::cout << "name is " << fname.data() << std::endl;
  for (unsigned int i = 0; i < mImages.size(); i++)
  {
    if (fname.compare(mImages[i]->name())==0)
      {
//       std::cout << "found index" << i << std::endl;
      mSelectedImage = i;
      found = true;
      }
  }
  return found;
}


void imageStack::clearSelection()
{
  mSelectedImage = -1;
}


void imageStack::resetFilters()
{
  //  for (unsigned int i = 0; i < mImages.size(); i++)
  //   {
  //   mImages[i]->resetModified();
  //  }
}
