#include "imageStack.h"

namespace wse {

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
  if (!img->read(fname))
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
    {    return mImages[mSelectedImage];  }
  else
    return NULL;
}

const Image *imageStack::selectedImage() const
{
  if (mSelectedImage >= 0)
    {  return mImages[mSelectedImage];   }
  else
    return NULL;
}

Image *imageStack::image(unsigned int i)
{
  if (i >= 0 && i < mImages.size())
    {    return mImages[i];    }
  else return NULL;
}

const Image *imageStack::image(unsigned int i) const
{
  if (i >= 0 && i < mImages.size())
    {    return mImages[i];    }
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
  {    return mImages[i]->name();  }
  return QString();
}


QString imageStack::selectedName()
{
  if (mSelectedImage >= 0)
  {    return mImages[mSelectedImage]->name();  }
  return QString();
}


bool imageStack::setSelectedByName(QString fname)
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
  
} // end namespace wse
