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
  \file    capture_generator.h
  \brief   C++ Interface: CaptureGenerator
  \author  Stefan Zickler, (C) 2009
*/
//========================================================================

#ifndef CAPTUREGENERATOR_H
#define CAPTUREGENERATOR_H

#include "captureinterface.h"
#include <string>
#include "VarTypes.h"
#include "framecounter.h"
#include "framelimiter.h"
#include "image.h"
#ifndef VDATA_NO_QT
  #include <QMutex>
#else
  #include <pthread.h>
#endif


#ifndef VDATA_NO_QT
  #include <QMutex>
  //if using QT, inherit QObject as a base
class CaptureGenerator : public QObject, public CaptureInterface
#else
class CaptureGenerator : public CaptureInterface
#endif
{
#ifndef VDATA_NO_QT
  Q_OBJECT
/*   public slots: */
/*   void changed(VarType * group); */
  protected:
  QMutex mutex;
  public:
#endif

protected:
  bool is_capturing;
  RawImage result;
  FrameLimiter limit;
  //processing variables:
  VarStringEnum * v_colorout;

  VarList * capture_settings;
  VarList * conversion_settings;

  VarInt * v_width;
  VarInt * v_height;
  VarDouble * v_framerate;
  VarBool * v_test_image;
  
public:
#ifndef VDATA_NO_QT
  CaptureGenerator(VarList * _settings, QObject * parent=0);
  void mvc_connect(VarList * group);
#else
  CaptureGenerator(VarList * _settings);
#endif
  ~CaptureGenerator();
    
  virtual bool startCapture();
  virtual bool stopCapture();
  virtual bool isCapturing() { return is_capturing; };
  
  virtual RawImage getFrame();
  virtual void releaseFrame();
   
  void cleanup();

  virtual bool copyAndConvertFrame(const RawImage & src, RawImage & target);
  virtual string getCaptureMethodName() const;
};

#endif
