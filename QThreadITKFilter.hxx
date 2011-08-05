//
// Josh Cates
// SCI Institute
// University of Utah
//
#include <QThread>

namespace itk {

/** This class is a Qt worker thread class for executing a single itk image-to-image filter type.
    Use the class as follows:
    (1) Create an instance using your favorite itk image-to-image filter as the template parameter.
    (2) Set the ITK filter inputs and outputs and necessary parameters as usual.
    (3) Execute the start() method
    (4) Enjoy ITK!
*/
template <class itkfilter>
class ITK_EXPORT QThreadITKFilter : public QThread, public itkfilter
{
 public:
  /** This method is executed when the thread is spawned -- call start() on this class from the 
      calling program. Thread is cleaned up when this function returns. */
  void run()
  {
    this->Update();
  }	   
  
  /** Assign the input image (required before calling start()) */
  //  void setInput(const itkfilter::InputImageType *img)
  //  { mInput = img; }
  
  /** Assign the output image (required before calling start()) */
  // void setOutput(itkfilter::OutputImageType *img)
  // { mOutput = img;  }
  
  // protected:
      //  typename itkfilter::InputImageType::ConstPointer mInput;
      // typename itkfilter::OutputImageType::Pointer mOutput;


}; // end class

} // end namespace itk
