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
  \file    capture_bluefox3.h
  \brief   C++ Interface: CaptureBlueFox3
  \author  Nicolai Ommer, (C) 2019
*/
//========================================================================

#ifndef CAPTURE_BLUEFOX3_H
#define CAPTURE_BLUEFOX3_H
#include "captureinterface.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "VarTypes.h"
#include <mvIMPACT_CPP/mvIMPACT_acquire.h>
#include <mvIMPACT_CPP/mvIMPACT_acquire_GenICam.h>

//#include "conversions.h"
#ifndef VDATA_NO_QT
  #include <QMutex>
#else
  #include <pthread.h>
#endif

using namespace mvIMPACT::acquire;

/*!
  \class  CaptureBlueFox3
  \brief  A capture class for Matrix-Vision BlueFox3 cameras
  \author Nicolai Ommer, (C) 2019

  This class provides the ability to use and configure Matrix-Vision
  BlueFox3 cameras using the mvIMPACT driver.

  If you find your camera not working correctly, or discover a bug,
  please inform the author, as we are aiming for complete camera
  coverage.
*/
#ifndef VDATA_NO_QT
  #include <QMutex>
  //if using QT, inherit QObject as a base
class CaptureBlueFox3 : public QObject, public CaptureInterface
#else
class CaptureBlueFox3 : public CaptureInterface
#endif
{
#ifndef VDATA_NO_QT
  Q_OBJECT
  
  public slots:
  void changed(VarType * group);
  
  protected:
  QMutex mutex;
  
  public:
#endif

protected:
  bool is_capturing;

  //capture variables:
  VarInt    * v_cam_bus;
  VarStringEnum * v_colorout;

  VarList * capture_settings;
  
  // BlueFox3 specific data
  DeviceManager* pDevMgr;
  Device* pDevice;
  FunctionInterface* pFI;
  int lastRequestNr;

  unsigned int cam_id;

public:
#ifndef VDATA_NO_QT
  explicit CaptureBlueFox3(VarList * _settings= nullptr, int default_camera_id=0, QObject * parent=nullptr);
#else
    CaptureBlueFox3(VarList * _settings=0,int default_camera_id=0);
#endif
  ~CaptureBlueFox3() override;

  /// Initialize the interface and start capture
  bool startCapture() override;

  /// Stop Capture
  bool stopCapture() override;

  bool isCapturing() override { return is_capturing; };

  /// this gives a raw-image with a pointer directly to the video-buffer
  /// Note that this pointer is only guaranteed to point to a valid
  /// memory location until releaseFrame() is called.
  RawImage getFrame() override;

  void releaseFrame() override;

  bool resetBus() override;

  void readAllParameterValues() override;

  bool copyAndConvertFrame(const RawImage & src, RawImage & target) override;

  string getCaptureMethodName() const override;
};

#endif
