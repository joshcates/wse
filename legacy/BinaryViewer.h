/*
 * vtkFlRenderWindowInteractor - class to enable VTK to render to and interact
 * with a FLTK window.
 * 
 * Copyright (c) 2002 Charl P. Botha <cpbotha@ieee.org> http://cpbotha.net/
 * Based on original code and concept copyright (c) 2000,2001 David Pont
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 * 
 * See the .cxx for more notes.
 * 
 * $Id: BinaryViewer.h,v 1.2 2003-07-25 18:09:25 darbyb Exp $
 */

#ifndef _BinaryViewer_h
#define _BinaryViewer_h

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include "vtkFlImageViewer.h"

class BinaryViewer : public vtkFlImageViewer {
 public:
   // Fl_Gl_Window overrides
   int  handle( int event );

 public:
   // constructors
   BinaryViewer();
   // fltk required constructor
   BinaryViewer( int x, int y, int w, int h, const char *l="");
   // vtk ::New()
   static BinaryViewer* New();
   // destructor
   ~BinaryViewer( void );

  };

#endif
