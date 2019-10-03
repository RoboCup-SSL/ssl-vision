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

#pragma once

#include "captureinterface.h"
#include "VarTypes.h"
#include <mutex>

  #include <QMutex>


//if using QT, inherit QObject as a base
class CaptureSplitter : public QObject, public CaptureInterface
{
  Q_OBJECT
/*   public slots: */
  protected:
  QMutex mutex;
  public:

protected:
  bool is_capturing;

  VarDouble* relative_height_offset;
  VarDouble* relative_width_offset;
  VarDouble* relative_width;
  VarDouble* relative_height;

  RawImage* full_image;
  RawImage* image_buffer;
  std::mutex full_image_arrived_mutex;
  std::mutex frame_processed_mutex;

public:
  CaptureSplitter(VarList * _settings, int default_camera_id, QObject * parent=nullptr);
  ~CaptureSplitter() override;

  bool startCapture() override;
  bool stopCapture() override;
  bool isCapturing() override { return is_capturing; };
  
  RawImage getFrame() override;
  void releaseFrame() override;
   
  void cleanup();

  bool copyAndConvertFrame(const RawImage & src, RawImage & target) override;
  string getCaptureMethodName() const override;

  void onNewFrame(RawImage* image);
  void waitUntilFrameProcessed();
};

