//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    capture_generator.cpp
  \brief   C++ Implementation: CaptureGenerator
  \author  Stefan Zickler, (C) 2009
*/
//========================================================================

#include "capture_generator.h"
#include "conversions.h"


#ifndef VDATA_NO_QT
CaptureGenerator::CaptureGenerator ( VarList * _settings, QObject * parent ) : QObject ( parent ), CaptureInterface ( _settings )
#else
CaptureGenerator::CaptureGenerator ( VarList * _settings ) : CaptureInterface ( _settings )
#endif
{
  is_capturing=false;

  settings->addChild ( conversion_settings = new VarList ( "Conversion Settings" ) );
  settings->addChild ( capture_settings = new VarList ( "Capture Settings" ) );

  //=======================CONVERSION SETTINGS=======================
  conversion_settings->addChild ( v_colorout=new VarStringEnum ( "convert to mode",Colors::colorFormatToString ( COLOR_YUV422_UYVY ) ) );
  v_colorout->addItem ( Colors::colorFormatToString ( COLOR_RGB8 ) );
  v_colorout->addItem ( Colors::colorFormatToString ( COLOR_YUV422_UYVY ) );

  //=======================CAPTURE SETTINGS==========================
  capture_settings->addChild ( v_framerate = new VarDouble ( "Framerate (FPS)", 60.0 ) );
  capture_settings->addChild ( v_width = new VarInt ( "Width (pixels)", 780 ) );
  capture_settings->addChild ( v_height = new VarInt ( "Height (pixels)", 580 ) );
  capture_settings->addChild ( v_test_image = new VarBool ( "Generate Color Test Image", false ) );
}

CaptureGenerator::~CaptureGenerator()
{
}

bool CaptureGenerator::stopCapture()
{
  cleanup();
  return true;
}

void CaptureGenerator::cleanup()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  is_capturing=false;
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

bool CaptureGenerator::startCapture()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  limit.init ( v_framerate->getDouble() );
  is_capturing=true;


#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

bool CaptureGenerator::copyAndConvertFrame ( const RawImage & src, RawImage & target )
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  ColorFormat output_fmt = Colors::stringToColorFormat ( v_colorout->getSelection().c_str() );
  ColorFormat src_fmt=src.getColorFormat();

  if ( target.getData() ==0 ) {
    target.allocate ( output_fmt, src.getWidth(), src.getHeight() );
  } else {
    target.ensure_allocation ( output_fmt, src.getWidth(), src.getHeight() );
  }
  target.setTime ( src.getTime() );

  if ( output_fmt == src_fmt ) {
    if ( src.getData() != 0 ) memcpy ( target.getData(),src.getData(),src.getNumBytes() );
  } else if ( src_fmt == COLOR_RGB8 && output_fmt == COLOR_YUV422_UYVY ) {
    if ( src.getData() != 0 ) {
      dc1394_convert_to_YUV422 ( src.getData(), target.getData(), src.getWidth(), src.getHeight(),
                                 DC1394_BYTE_ORDER_UYVY, DC1394_COLOR_CODING_RGB8, 8 );
    }
  } else {
    fprintf ( stderr,"Cannot copy and convert frame...unknown conversion selected from: %s to %s\n",
              Colors::colorFormatToString ( src_fmt ).c_str(),
              Colors::colorFormatToString ( output_fmt ).c_str() );
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  }
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

RawImage CaptureGenerator::getFrame()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  limit.waitForNextFrame();
  result.setColorFormat ( COLOR_RGB8 );
  result.setTime ( GetTimeSec() );
  result.allocate ( COLOR_RGB8,v_width->getInt(),v_height->getInt() );
  rgbImage img;
  img.fromRawImage(result);

  if (v_test_image->getBool()) {
    int w = result.getWidth();
    int h = result.getHeight();
    int n_colors = 8;
    int slice_width = w/n_colors;
    rgb color=RGB::Black;
    rgb color2;
    int gradient=h/256;
    for (int x = 0 ; x < w; x++) {
      int c_idx= x/slice_width;
      if (c_idx==0) {
        color=RGB::White;
      } else if (c_idx==1) {
        color=RGB::Red;
      } else if (c_idx==2) {
        color=RGB::Green;
      } else if (c_idx==3) {
        color=RGB::Blue;
      } else if (c_idx==4) {
        color=RGB::Cyan;
      } else if (c_idx==5) {
        color=RGB::Pink;
      } else if (c_idx==6) {
        color=RGB::Yellow;
      } else if (c_idx==7) {
        color=RGB::Black;
      }
      for (int y = 0 ; y < h; y++) {
        color2.r= max(0,min(255,((int)color.r * (h-1-y))/h));
        color2.g= max(0,min(255,((int)color.g * (h-1-y))/h));
        color2.b= max(0,min(255,((int)color.b * (h-1-y))/h));
        img.setPixel(x,y,color2);
      }
    }
  } else {
    img.fillBlack();
  }


#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return result;
}

void CaptureGenerator::releaseFrame()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

string CaptureGenerator::getCaptureMethodName() const
{
  return "Image Generator";
}
