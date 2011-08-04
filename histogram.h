#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP

//std includes
#include <limits>
#include <cmath>
#include <vector>

//itk includes
#include "itkImage.h"
#include "itkImageRegionConstIterator.h"

//local includes
#include "exception.h"


#ifdef _WIN32
#include <float.h>
#define nextafter _nextafter
#endif

namespace wse {

  /**
   @class Histogram
   @brief This is a basic class for computing the intensity histogram of an ITK
   image.  Parameters are the image and the number of bins for the histogram.
   After construction, the object can be queried for bin information,
   frequencies, and basic image summary statistics.
   */
  template <class TImageType, class TMaskType=TImageType>// , class TFrequency = unsigned long>
  class Histogram
  {
  public:
    typedef TImageType ImageType;
    typedef TMaskType  MaskImageType;
    typedef typename ImageType::PixelType PixelType;
    
    /**
     * @param im - the image to be histogram'ed.
     * @param numBins - the number of bins to use.
     */
    Histogram(const typename ImageType::Pointer im, unsigned int numBins);
    /**
     * @param im - the image to be histogram'ed.
     * @param numBins - the number of bins to use.
     * @param filterMin - filter out any values less than this value.
     * @param filterMax - filter out any values greater than this value.
     */
    Histogram(const typename ImageType::Pointer im, unsigned int numBins,
              PixelType filterMin, PixelType filterMax);
    /**
     * @param im - the image to be histogram'ed.
     * @param mask - the image mask defining the region of interest
     * @param numBins - the number of bins to use.
     */
    Histogram(const typename ImageType::Pointer im,
              const typename MaskImageType::Pointer mask,
              unsigned int numBins);
    /**
     * @param im - the image to be histogram'ed.
     * @param mask - the image mask defining the region of interest
     * @param numBins - the number of bins to use.
     * @param filterMin - filter out any values less than this value.
     * @param filterMax - filter out any values greater than this value.
     */
    Histogram(const typename ImageType::Pointer im,
              const typename MaskImageType::Pointer mask,
              unsigned int numBins,
              PixelType filterMin, 
              PixelType filterMax);
    
    // Implemented to properly copy std::vector members
    const Histogram& operator=(const Histogram &in)
    {
      m_min = in.m_min;
      m_max = in.m_max;
      m_binWidth = in.m_binWidth;
      m_mean = in.m_mean;
      m_stdev = in.m_stdev;
      m_pixelCount = in.m_pixelCount;
      m_bins = in.m_bins;
      m_hist = in.m_hist;
      m_filterMin = in.m_filterMin;
      m_filterMax = in.m_filterMax;
      
      return *this;
    }
    Histogram(const Histogram &h)
    {  this->operator=(h); }
    
    /**
     * @return the histogram frequency counts.
     */
    const std::vector<unsigned long> &frequencies() const { return m_hist; }
    std::vector<unsigned long> &frequencies() { return m_hist; }
    
    /**
     * @return the bin boundaries (minimum boundary side)
     */
    const std::vector<double> &bins() const { return m_bins; }
    std::vector<double> &bins() { return m_bins; }
    
    /**
     * @return the width of the bins in pixel units
     * For example, a histogram with 10 bins containing values from 0 to 100 will
     * have binWidth of 10.
     */
    double binWidth() const { return m_binWidth;}
    
    /**
     * @return the min value in the image
     */
    PixelType min() const { return m_min; }
    
    /**
     * @return the max value in the image.
     */
    PixelType max() const { return m_max; }
    
    /**
     * @return the mean value in the image
     */
    double mean() const {return m_mean; }
    
    /**
     * @return the standard deviation of values in the image
     */
    double stdev() const {return m_stdev; }
    
    /**
     * @return the number of bins
     */
    unsigned int numBins() const { return m_bins.size(); }
    
    /**
     * @return the number of pixels counted 
     */
    unsigned long pixelCount() const { return m_pixelCount; }
    
    /**
     * @return the total number of pixels in the image
     */
    unsigned long totalPixels() const { return m_totalPixels; }
    
    /**
     * @return the minimum value included in histogram calculation.
     */
    PixelType filterMin() const { return m_filterMin; }
    /**
     * @return the maximum value included in histogram calculation.
     */
    PixelType filterMax() const { return m_filterMax; }
    
  private:
    /** Intentionally private.*/
    Histogram(); 
    
    /**
     * method that all constructors call to build the histogram.
     * @param im - the original image.
     * @param mask - the mask image.
     * @param useThreshold - whether to threshold the pixels or not.
     */
    void buildHistogram(const typename ImageType::Pointer im,
                        const typename MaskImageType::Pointer mask,
                        bool useThreshold);
    
    /** Maximum pixel value found in the image. */
    PixelType m_max;
    /** Minimum pixel value found in the image. */
    PixelType m_min;
    
    /** Mean value of pixelCount in the image. */
    double m_mean;
    /** Standard deviation of the pixel values in the image.*/
    double m_stdev;
    
    /** The total number of pixelCount counted in the image. */
    unsigned long m_pixelCount;
    
    /** The total number of pixels in the image. */
    unsigned long m_totalPixels;
    
    /** frequency counts for each bin. */
    std::vector<unsigned long> m_hist;
    
    /** maximum value in each bin. */
    std::vector<double> m_bins;
    
    /** size of each bin. */
    double m_binWidth;
    
    /** Histogram only counts values >= m_filterMin. */
    PixelType m_filterMin;
    /** Histogram only counts values <= m_filterMax. */
    PixelType m_filterMax;  
  };
  
  template <class TImageType, class TMaskImageType>
  Histogram<TImageType,TMaskImageType>::Histogram(const typename ImageType::Pointer im, unsigned int numBins)
  :m_max(0),
  m_min(0),
  m_mean(0),
  m_stdev(0),
  m_pixelCount(0),
  m_totalPixels(0),
  m_hist(numBins),
  m_bins(numBins),
  m_binWidth(0),
  m_filterMin(0),
  m_filterMax(0)
  {
    buildHistogram(im , MaskImageType::New() , false);
  }
  
  template <class TImageType, class TMaskImageType>
  Histogram<TImageType,TMaskImageType>::Histogram(const typename ImageType::Pointer im, unsigned int numBins,
                                                  PixelType filterMin, PixelType filterMax)
  :m_max(0),
  m_min(0),
  m_mean(0),
  m_stdev(0),
  m_pixelCount(0),
  m_totalPixels(0),
  m_hist(numBins),
  m_bins(numBins),
  m_binWidth(0),
  m_filterMin(filterMin),
  m_filterMax(filterMax)
  {
    buildHistogram(im , MaskImageType::New() , true);
  }
    
  template <class TImageType, class TMaskImageType>
  Histogram<TImageType,TMaskImageType>::Histogram(const typename ImageType::Pointer im,
                                                  const typename MaskImageType::Pointer mask,
                                                  unsigned int numBins)
  :m_max(0),
  m_min(0),
  m_mean(0),
  m_stdev(0),
  m_pixelCount(0),
  m_totalPixels(0),
  m_hist(numBins),
  m_bins(numBins),
  m_binWidth(0),
  m_filterMin(0),
  m_filterMax(0)
  {
    buildHistogram(im , mask , false);
  }
  
  template <class TImageType, class TMaskImageType>
  Histogram<TImageType,TMaskImageType>::Histogram(const typename ImageType::Pointer im,
                                                  const typename MaskImageType::Pointer mask,
                                                  unsigned int numBins,
                                                  PixelType filterMin, 
                                                  PixelType filterMax)
  :m_max(0),
  m_min(0),
  m_mean(0),
  m_stdev(0),
  m_pixelCount(0),
  m_totalPixels(0),
  m_hist(numBins),
  m_bins(numBins),
  m_binWidth(0),
  m_filterMin(filterMin),
  m_filterMax(filterMax)
  {
    buildHistogram(im , mask, true);
  }
  
  
  template <class TImageType, class TMaskImageType>
  void Histogram<TImageType,TMaskImageType>::buildHistogram(const typename ImageType::Pointer im,
                      const typename MaskImageType::Pointer mask,
                      bool useThreshold)
  {    
    typedef typename itk::ImageRegionConstIterator<ImageType> ConstIteratorType;
    typedef typename itk::ImageRegionConstIterator<MaskImageType> MaskConstIteratorType;
    
    typename ImageType::SizeType imageSize = im->GetLargestPossibleRegion().GetSize();
    typename ImageType::SizeType maskSize = mask->GetLargestPossibleRegion().GetSize();
    m_totalPixels = 1;
    for(unsigned int i = 0; i < imageSize.GetSizeDimension(); ++i)
    {
      m_totalPixels *= imageSize[i];
    }
    
    // Some basic error checks -- the dimensions and image sizes should be same for
    // mask and data image.
    if (maskSize[0] != 0)
    {
      if (ImageType::ImageDimension != MaskImageType::ImageDimension)
      {
        throw Exception("Histogram -- The data and mask images have different dimensionalities.");
      }
      for (unsigned int i = 0; i < ImageType::ImageDimension; i++)
      {
        if ( imageSize[i] != maskSize[i] )
        {
          throw Exception("Histogram -- The data and mask images have different sizes.");
        }
      }
    }
    
    // First pass: Gather image statistics
    MaskConstIteratorType mit(mask, mask->GetRequestedRegion());
    ConstIteratorType it( im , im->GetRequestedRegion() );
    it.GoToBegin();
    mit.GoToBegin();
    m_min = std::numeric_limits<PixelType>::max();
    m_max = std::numeric_limits<PixelType>::min();
    double sum = 0;
    for( ; !it.IsAtEnd(); ++it)
    {
      PixelType pixel = it.Get();
      if ((maskSize[0] == 0 || mit.Get() != 0) &&
          (!useThreshold || (pixel > m_filterMin && pixel < m_filterMax))) 
      {
        if (pixel < m_min) m_min = pixel;
        else if (pixel > m_max) m_max = pixel;
        
        sum += pixel;
        // catch overflow of summation
        if( sum > std::numeric_limits<double>::max() ) { throw Exception("Histogram - Overflow in mean calculation!  Image has too many pixels."); }
        
        // Handle case with too many pixels
        if (m_pixelCount == std::numeric_limits<unsigned long>::max())
        { 
          throw Exception("Histogram - Overflow in pixel counting!  Image has too many pixels.");
        }
        else
        {
          m_pixelCount++;
        }
      }
      
      if (maskSize[0] != 0) ++mit;
    }
    
    // Handle zero pixel count
    if (m_pixelCount == 0) {
      //throw Exception("Histogram - No pixels found!");
      m_mean = 0;
    } else {
      m_mean = sum / m_pixelCount;
    }
    
    // Set up the bins
    m_binWidth = static_cast<double>(m_max-m_min) / m_bins.size();
    for(unsigned int i=0; i < m_bins.size(); ++i)
    {
      m_bins[i] = m_min + m_binWidth * i;
    }
    
    // Second pass through image: fill in the bins, compute stdev
    for( mit.GoToBegin(), it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      PixelType pixel = it.Get();
      if ((maskSize[0] == 0 || mit.Get() != 0) &&
          (!useThreshold || (pixel > m_filterMin && pixel < m_filterMax))) 
      {
        
        // note: extra divisions, but controls for overflow.  maybe not a problem?
        if (m_pixelCount == 0) {
          m_stdev = 0;
        } else {
          m_stdev += ((pixel - m_mean) * (pixel - m_mean)) / m_pixelCount;
        }
        
        // Map to appropriate bin, assuming equally-spaced bins.  Maximum value is
        // mapped to the last bin.
        if (pixel == m_max)
        {
          m_hist[m_bins.size()-1]++;
        }
        else
        {
          //unsigned int b = static_cast<unsigned int>(floor( (p - m_min) / m_binWidth) );
          //m_hist[b]++;
          
          // binary search - a la matlab (see /Applications/MATLAB74/toolbox/matlab/datafun/hist.m,histc.c)
          unsigned int k0 = 0;
          unsigned int k1 = m_bins.size() - 1;
          unsigned int k = (k0 + k1) / 2;
          
          while (k0 < k1 - 1) 
          {
            if (pixel >= nextafter(m_bins[k], m_bins[k]+1))
            {
              k0 = k;
            }
            else
            {
              k1 = k;
            }
            k = (k0+k1)/2;
          }
          k = k0;
          
          m_hist[k]++;        
        }
      }
      
      if (maskSize[0] != 0) ++mit;
    }
    m_stdev = sqrt(m_stdev);
  }  
  
}//end namespace wse

#endif
