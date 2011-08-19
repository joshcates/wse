#ifndef __wseException_h
#define __wseException_h

#include <string>
#include <ostream>

namespace wse
{

/** \class Exception
    A simple exception class that may be thrown and caught.  The optional text
    string provides additional information about the error.
*/
class Exception
{
public:
  Exception(std::string s) : info(s) {}
  Exception() { info = "no additional information available"; }
  std::string info;
};

inline std::ostream& operator<<(std::ostream& s, const Exception &f)
{ s << "Wse exception: " << f.info; return s; }

} // end namespace wse

#endif // __wseException_h
