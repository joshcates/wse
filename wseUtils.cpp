#include "wseUtils.h"
#include <vtkImageData.h>

namespace wse {
//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
// QImage IplImageToQImage(const IplImage *iplImage)
// {
//   if (iplImage->depth != IPL_DEPTH_8U)
//   {
//     throw "Error: Only images with depths of 8 are supported!\n";
//   }
  
//   uchar *qImageBuffer = NULL;
//   int width = iplImage->width;
//   int height = iplImage->height;
//   int widthStep = iplImage->widthStep;
//   QImage::Format format;

//   if (iplImage->nChannels == 1)
//   {
//     qImageBuffer = new uchar[width * height];
//     uchar *qptr = qImageBuffer;
//     const uchar *iptr = (const uchar *)iplImage->imageData;
//     for (int y = 0; y < height; y++)
//     {
//       memcpy(qptr, iptr, width);
//       qptr += width;
//       iptr += widthStep;
//     }
//     format = QImage::Format_Indexed8;
//   }
//   else if (iplImage->nChannels == 3)
//   {
//     qImageBuffer = new uchar[width * height * 4];
//     uchar *qptr = qImageBuffer;
//     const uchar *iptr = (const uchar *)iplImage->imageData;
//     for (int y = 0; y < height; y++)
//     {
//       for (int x = 0; x < width; x++)
//       {
//         qptr[0] = iptr[0];
//         qptr[1] = iptr[1];
//         qptr[2] = iptr[2];
//         qptr[3] = 0;
        
//         qptr+=4;
//         iptr+=3;
//       }
//       iptr += widthStep - 3 * width;
//     }
//     format = QImage::Format_RGB32;
//   }
//   else
//   {
//     throw "Error: Only images with 1 or 3 channels are supported!\n";
//   }

//   QImage qimage(qImageBuffer, width, height, format);
//   return qimage;
// }

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
bool QImageToVTKImage(const QImage &img, vtkImageData* vtkimage)
{
  int height = img.height();
  int width = img.width();
  int numcomponents = img.hasAlphaChannel() ? 4 : 3;
  
  vtkimage->SetWholeExtent(0, width-1, 0, height-1, 0, 0); 
  vtkimage->SetSpacing(1.0, 1.0, 1.0);
  vtkimage->SetOrigin(0.0, 0.0, 0.0);
  vtkimage->SetNumberOfScalarComponents(numcomponents);
  vtkimage->SetScalarType(VTK_UNSIGNED_CHAR);
  vtkimage->SetExtent(vtkimage->GetWholeExtent());
  vtkimage->AllocateScalars();
  for(int i=0; i<height; i++)
  {
    unsigned char* row;
    row = static_cast<unsigned char*>(vtkimage->GetScalarPointer(0, height-i-1, 0));
    const QRgb* linePixels = reinterpret_cast<const QRgb*>(img.scanLine(i));
    for(int j=0; j<width; j++)
    {
      const QRgb& col = linePixels[j];
      row[j*numcomponents] = qRed(col);
      row[j*numcomponents+1] = qGreen(col);
      row[j*numcomponents+2] = qBlue(col);
      if(numcomponents == 4)
      {
        row[j*numcomponents+3] = qAlpha(col);
      }
    }
  }
  return true;
}

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
void QColorToVTKColor(const QColor &qc, double *c)
{
  c[0] = double(qc.red())/255.0;
  c[1] = double(qc.green())/255.0;
  c[2] = double(qc.blue())/255.0;
}

#ifdef WIN32

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

// maximum number of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;

void RedirectIOToConsole() {
  int hConHandle;
  long lStdHandle;

  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;

  // allocate a console for this app
  AllocConsole();

  // set the screen buffer to be big enough to let us scroll text
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  //coninfo.dwSize.Y = MAX_CONSOLE_LINES;

  coninfo.dwSize.X = 10000;
  coninfo.dwSize.Y = 10000;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);


  SMALL_RECT windowSize = {0, 0, 79, 49};

  // Change the console window size:
  SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &windowSize);


//  SetConsoleWindowInfo()

  // redirect unbuffered STDOUT to the console
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "w" );
  *stdout = *fp;
  setvbuf( stdout, NULL, _IONBF, 0 );

  // redirect unbuffered STDIN to the console
  lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "r" );
  *stdin = *fp;
  setvbuf( stdin, NULL, _IONBF, 0 );

  // redirect unbuffered STDERR to the console
  lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "w" );
  *stderr = *fp;
  setvbuf( stderr, NULL, _IONBF, 0 );

  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
  // point to console as well
  ios::sync_with_stdio(true);

}

#endif
} // end namespace
