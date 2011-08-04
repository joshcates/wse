#include "wse.h"
#include <QtGui/QApplication>

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

  wse::wseApplication a(argc, argv);
  wse::wseGUI w;

  a.loadStyleSheet(":/WSE/qss/default.qss");
  
  QPixmap pixmap(":/WSE/Resources/splash2.png");
  QSplashScreen splash(pixmap,Qt::WStyle_StaysOnTop);
  splash.show();
  sleep(1.0);
  a.processEvents();
  w.show();
  w.raise();
  splash.finish(&w);
  
  // The following sets the paths for dynamic library loading
  // (plugins) at runtime.  The addLibraryPath command does not appear
  // to work in Qt 4.7.  The following will put the package distribution
  // plugins directory (on macs) at the top of the existing default list of paths.
  // Won't hurt anything if there is no plugins directory. --jc
  QStringList ldpath = a.libraryPaths();
  ldpath.push_front(QString(a.applicationDirPath() + "/plugins"));
  a.setLibraryPaths(ldpath);

  // This is how it is supposed to work according to Qt documentation:
  // a.addLibraryPath(a.applicationDirPath() + "/plugins");
  
  // Debug -- print out the library paths
  //  QStringList::ConstIterator it = a.libraryPaths().constBegin();
  // while (it != a.libraryPaths().constEnd())
  //   {
  //	std::cout << "library path = " << it->toStdString() << std::endl;
  //	it++;
  //   }

  return a.exec();
}
