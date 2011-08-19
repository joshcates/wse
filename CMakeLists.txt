# jc -- For more information building and packaging with CMake and Qt, see the
# following URL:  http://www.cmake.org/Wiki/BundleUtilitiesExample
# See also the following thread:
#  http://www.mail-archive.com/paraview@paraview.org/msg05234.html
PROJECT( WSE )
cmake_minimum_required(VERSION 2.8)  

#Match everything when analyzing dependencies
INCLUDE_REGULAR_EXPRESSION("^.*$")
OPTION (BUILD_SHARED_LIBS "Build shared libraries" OFF)
OPTION (BUILD_TESTS "Build Wse test applications." ON)
OPTION (INSTALL_TEST_FILES "Include Wse test images and help files in the install." OFF)
OPTION (INSTALL_SOURCE  "Install Wse source code." OFF)

set(CMAKE_BUILD_TYPE Release)

FIND_PACKAGE(VTK REQUIRED)
IF (VTK_FOUND)
 INCLUDE( ${USE_VTK_FILE} )
ENDIF(VTK_FOUND)

FIND_PACKAGE(ITK REQUIRED)
IF (ITK_FOUND)
 INCLUDE (${USE_ITK_FILE})
ENDIF(ITK_FOUND)

include(FindQwt.cmake)
if(QWT_FOUND)
  include_directories( ${QWT_INCLUDE_DIR})
  link_directories( ${QWT_LIBRARY} )
endif(QWT_FOUND)

# Build the watersheds library
set(WSE_WATERSHEDS_SRCS
wseWatersheds/vtkBinaryVolume.cxx
wseWatersheds/vtkBinaryVolumeLogic.cxx
wseWatersheds/vtkBoundingBox.cxx
wseWatersheds/vtkLookupTableEquivalencyHash.cxx
wseWatersheds/vtkPatchedImageReader.cxx
wseWatersheds/vtkPatchedLookupTable.cxx
wseWatersheds/vtkWSBoundingBoxManager.cxx
wseWatersheds/vtkWSLookupTableManager.cxx
)

add_library(wseWatersheds ${WSE_WATERSHEDS_SRCS})

include_directories(wseGraphics)

SET( WSE_SRCS
     main.cpp
     wse.cpp
     wseProcessing.cpp
     image.cpp
     widgets.cpp
     histogramWidget.cpp
     utils.cpp
     imageStack.cpp
     wseGraphics/SliceViewer.cc
#     wseGraphics/IsoRenderer.cpp
     histogram.cpp
)

SET( WSE_MOC_HDRS
     wse.h
     widgets.h
     histogramWidget.h
     imageStack.h
)

SET ( WSE_HDRS
     histogram.h
     utils.h
     wseGraphics/SliceViewer.h
     image.h
     colorScheme.h
     colorMaps.h
     wseGraphics/IsoRenderer.h
)

SET( WSE_UIS
     wse.ui
)

SET( WSE_RCS
     wse.qrc
)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

#ADD_DEFINITIONS( -Wall )

# by default only QtCore and QtGui modules are enabled
# other modules must be enabled like this:
SET( QT_USE_QT3SUPPORT TRUE )   
SET( QT_USE_QTXML TRUE )
SET( QT_USE_QTSVG TRUE )
SET(QT_USE_QTWEBKIT TRUE)

#--------------------------------------------------------------------------------
# This sets the windows build that will need the special winmain@16 call. Qt provides
#  this for us in the qtmain.lib file. Using this cmake code will ensure we have it
#  linked into our build. Not needed on Unix/OS X/Linux which is why we have the 
#  IF(WIN32) conditional.
IF (WIN32)
    SET (QT_USE_QTMAIN TRUE)
ENDIF (WIN32)

# this command finds Qt4 libraries and sets all required variables
# note that it's Qt4, not QT4 or qt4
FIND_PACKAGE( Qt4 REQUIRED )

# add some useful macros and variables
# (QT_USE_FILE is a variable defined by FIND_PACKAGE( Qt4 ) that contains a path to CMake script)
INCLUDE( ${QT_USE_FILE} )

# this command will generate rules that will run rcc on all files from WSE_RCS
# in result WSE_RC_SRCS variable will contain paths to files produced by rcc
QT4_ADD_RESOURCES( WSE_RC_SRCS ${WSE_RCS} )

# this will run uic on .ui files:
QT4_WRAP_UI( WSE_UI_HDRS ${WSE_UIS} )

# and finally this will run moc:
QT4_WRAP_CPP( WSE_MOC_SRCS ${WSE_MOC_HDRS} )

# we need this to be able to include headers produced by uic in our code
# (CMAKE_BINARY_DIR holds a path to the build directory)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# No actual compiled code in these yet, so they are commented out.  
#ADD_LIBRARY(CARMANumerics quantification.cpp histogram.cpp)


# optionally build the test files
IF ( BUILD_TESTS )

  INCLUDE(CTest)
  ENABLE_TESTING()

#  ADD_TEST(QuantTest QuantTest "${CMAKE_CURRENT_SOURCE_DIR}")
#  ADD_EXECUTABLE(QuantTest test/quanttest.cpp)
#  TARGET_LINK_LIBRARIES(QuantTest ${ITK_LIBRARIES})

ENDIF(BUILD_TESTS)

# For Apple set the icns file containing icons
IF(APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE WSE.icns) 
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/WSE.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  SET(WSE_SRCS ${WSE_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/WSE.icns)
ENDIF(APPLE)

if(MSVC)
  SOURCE_GROUP(moc_sources FILES ${WSE_MOC_SRCS})
endif(MSVC)


# here we instruct CMake to build WSE executable from all of the source files

ADD_EXECUTABLE( WSE MACOSX_BUNDLE WIN32 
                 ${WSE_SRCS}
                 ${WSE_HDRS}
                 ${WSE_MOC_SRCS} 
                 ${WSE_RC_SRCS} 
                 ${WSE_UI_HDRS} 
 		 ${WSE_MOC_HDRS}
)

TARGET_LINK_LIBRARIES( WSE ${QT_LIBRARIES} ITKAlgorithms
                         ITKBasicFilters 
                         ITKIO ITKNumerics ITKCommon QVTK vtkCommon
                         vtkGraphics vtkImaging vtkRendering vtkWidgets qwt)

# # INSTALLATION AND PACKAGING
# SET(plugin_dest_dir bin)
# SET(qtconf_dest_dir bin)
# SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/WSE")
# IF(APPLE)
#   SET(plugin_dest_dir WSE.app/Contents/MacOS)
#   SET(qtconf_dest_dir WSE.app/Contents/Resources)
#   SET(contents_dest_dir WSE.app/Contents)
#   SET(APPS "\${CMAKE_INSTALL_PREFIX}/WSE.app")
# ENDIF(APPLE)
# IF(WIN32)
#   SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/WSE.exe")
# ENDIF(WIN32)



# #--------------------------------------------------------------------------------
# # Install the WSE application, on Apple, the bundle is at the root of the
# # install tree, and on other platforms it'll go into the bin directory.
# INSTALL(TARGETS WSE 
#     BUNDLE DESTINATION . COMPONENT Runtime
#     RUNTIME DESTINATION bin COMPONENT Runtime
#     )

# #--------------------------------------------------------------------------------
# # Install needed Qt plugins by copying directories from the qt installation
# # One can cull what gets copied by using 'REGEX "..." EXCLUDE'
# INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

# #--------------------------------------------------------------------------------
# # Install needed Qt .nib files and others
# IF(APPLE)
# INSTALL(DIRECTORY "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources" DESTINATION ${contents_dest_dir} COMPONENT Runtime)
# ENDIF(APPLE)



# #--------------------------------------------------------------------------------
# # install a qt.conf file
# # this inserts some cmake code into the install script to write the file
# INSTALL(CODE "
#     file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
#     " COMPONENT Runtime)


# #--------------------------------------------------------------------------------
# # Use BundleUtilities to get all other dependencies for the application to work.
# # It takes a bundle or executable along with possible plugins and inspects it
# # for dependencies.  If they are not system dependencies, they are copied.

# # directories to look for dependencies
# SET(DIRS ${QT_LIBRARY_DIRS})

# # Now the work of copying dependencies into the bundle/package
# # The quotes are escaped and variables to use at install time have their $ escaped
# # An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# # Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# # over.
# INSTALL(CODE "
#     file(GLOB_RECURSE QTPLUGINS
#       \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
#     include(BundleUtilities)
#     fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
#     " COMPONENT Runtime)


# # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# # where CPackConfig.cmake is created by including CPack
# # And then there's ways to customize this as well
# set(CPACK_BINARY_DRAGNDROP ON)
# include(CPack)


# # Grab the source code files for installation
# #IF(INSTALL_SOURCE)
# # FILE(GLOB root_files "*.h" "*.txx" "*.cxx" "*.txt")
# # INSTALL(FILES ${root_files}          DESTINATION source)
# # FILE(GLOB test_files "test/*.h" "test/*.txx" "test/*.cxx" "test/*.txt")
# # INSTALL(FILES ${test_files}          DESTINATION source/test)
# #ENDIF(INSTALL_SOURCE)

# INSTALL(FILES README.txt DESTINATION .)

# # Install test images and help files in the bin directory
# #IF (INSTALL_TEST_FILES)
# #  SET(TEST_DATA   
# #  list of files here ...
# # )
# #  INSTALL(FILES ${TEST_DATA} DESTINATION bin)
# #ENDIF(INSTALL_TEST_FILES)


# #INSTALL(TARGETS WSE 
# #  RUNTIME DESTINATION bin
# #  LIBRARY DESTINATION lib
# #  ARCHIVE DESTINATION lib
# #)

# # Install the necessary shared ITK Dlls
# # TODO: Assumes the install or package is a Release build.  Need to add logic
# # to find debug vs shared builds and switch on Mac or Windows.s 
# #IF (BUILD_SHARED_LIBS)
# #  INSTALL(FILES ${ITK_LIBRARY_DIRS}/Release/ITKCommon.dll DESTINATION bin)
# #  INSTALL(FILES ${ITK_LIBRARY_DIRS}/Debug/ITKCommon.dll DESTINATION bin)
# #ENDIF(BUILD_SHARED_LIBS)


# # CPACK CONFIGURATION
# #INCLUDE(InstallRequiredSystemLibraries)

# #SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Wse Research Sandbox")
# #SET(CPACK_PACKAGE_VENDOR "CARMA Center Image Processing Division")
# #SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/PackageDescription.txt")
# #SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
# #SET(CPACK_PACKAGE_VERSION_MAJOR "0")
# #SET(CPACK_PACKAGE_VERSION_MINOR "1")
# #SET(CPACK_PACKAGE_VERSION_PATCH "0")
# #SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Wse ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PA#CKAGE_VERSION_PATCH}")
# #IF(WIN32 AND NOT UNIX)
# #  # There is a bug in NSI that does not handle full unix paths properly. Make
# #  # sure there is at least one set of four (4) backlashes.
# #  #  SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
# #  #  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
# #  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Wse Research Sandbox")
# #  #  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
# #  #  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
# #  #  SET(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
# #  SET(CPACK_NSIS_MODIFY_PATH ON)
# #ELSE(WIN32 AND NOT UNIX)
# #  #  SET(CPACK_STRIP_FILES "bin/MyExecutable")
# #    SET(CPACK_SOURCE_STRIP_FILES "")
# #ENDIF(WIN32 AND NOT UNIX)

# #SET(CPACK_PACKAGE_EXECUTABLES )
# #INCLUDE(CPack)
