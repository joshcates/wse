//
// Josh Cates
// SCI Institute
// University of Utah
//
#include <QThread>
#include <QDebug>

namespace itk {

/** This is an itk::Command useful for connecting ITK filters to
    QProgress bars.  The template class GUITYPE must define the method
    getProgressBar() which returns a pointer to a QProgressBar.

    To use this class: 

    itk::wseITKCallback<myGUIType>::Pointer mycmd = itk::wseITKCallback<myGUIType>::New();
    mycmd->setGui(myGuiPointer);
    myITKFilter->AddObserver(itk::ProgressEvent(), mycmd);

*/
template < class GUITYPE >
class wseITKCallback : public Command
{
public:
  typedef wseITKCallback Self;
  typedef Command Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  typedef WeakPointer<const Self>  ConstWeakPointer;
  itkNewMacro( wseITKCallback );
  
public:
  void setGui(GUITYPE *g)
  {
    mGui = g;
  }
  
  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event);
  }
  
  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    itk::ProcessObject *processObject = (itk::ProcessObject*)object;
    if (typeid(event) == typeid(itk::ProgressEvent)) 
      {
	// std::cout << "ITK Progress event received from "
	//  << processObject->GetNameOfClass() << ". Progress is "
	//  << 100.0 * processObject->GetProgress() << " %." << std::endl;
	  mGui->getProgressBar()->setValue(100.0 * processObject->GetProgress());
      }
  }
  
 private:
  GUITYPE *mGui;

}; // end class wseITKCallback


/** This class is a Qt worker thread class for executing a single itk image-to-image filter type.

    Use the class as follows:
    (1) Create your favorite itk image-to-image filter pointer.
    (2) Set the ITK filter inputs and outputs and necessary parameters as usual.
    (3) Use setFilter method to pass your itk filter pointer to this class.
    (3) Execute the start() method.
    (4) Enjoy ITK!
*/
template <class itkfilter>
class ITK_EXPORT QThreadITKFilter : public QThread
{
 public:
  QThreadITKFilter() : mFiltering(false), mErrorFlag(false) {}

  /** This method is executed when the thread is spawned -- call start() on this class from the 
      calling program. Thread is cleaned up when this function returns. */
  void run()
  {
    mMutex.lock();
    mFiltering = true;
    mErrorFlag  = false;

    try
      {
	mFilter->UpdateLargestPossibleRegion();
      }
    catch (itk::ExceptionObject &e)
      {
	mErrorFlag = true;
	mErrorString = e.GetDescription();
      }

    mFiltering = false;
    mMutex.unlock();
  }	   
  
  /** Set/Get the itk::filter object*/
  const typename itkfilter::Pointer &filter() const 
  { return mFilter; }
  typename itkfilter::Pointer &filter()  
  { return mFilter; }
  void setFilter(itkfilter *p)
  {
    mMutex.lock();
    if (! mFiltering)
      {
	mFilter = p;
      }
    mMutex.unlock();
  }

  /** Returns true if the thread is executing and false otherwise. */
  bool isFiltering() const
  { 
    return mFiltering;
  }

  /** Returns true if the thread process returned an error. */
  bool errorFlag() const
  { return mErrorFlag; }

  /** Returns a string with any error information. */
  const QString &errorString() const
  { return mErrorString; }

 private:
  typename itkfilter::Pointer mFilter;
  bool mFiltering;
  bool mErrorFlag;
  QString mErrorString;
  QMutex mMutex;

}; // end class

} // end namespace itk
