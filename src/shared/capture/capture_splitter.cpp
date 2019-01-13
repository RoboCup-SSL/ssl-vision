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

#include "capture_splitter.h"

#ifndef VDATA_NO_QT
CaptureSplitter::CaptureSplitter(VarList * _settings, int default_camera_id, QObject * parent) : QObject(parent), CaptureInterface(_settings)
#else
CaptureSplitter::CaptureSplitter(VarList * _settings) : CaptureInterface(_settings)
#endif
{
  is_capturing=false;
  camera_id = default_camera_id;
  frame = new RawImage();
  nextFrame = new RawImage();
}

CaptureSplitter::~CaptureSplitter()
{
  delete frame;
  delete nextFrame;
}

bool CaptureSplitter::stopCapture()
{
  cleanup();
  return true;
}

void CaptureSplitter::cleanup()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif
  is_capturing=false;
#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
}

bool CaptureSplitter::startCapture()
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif

  is_capturing = true;

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

bool CaptureSplitter::copyAndConvertFrame(const RawImage & src, RawImage & target)
{
#ifndef VDATA_NO_QT
  mutex.lock();
#endif


  if(src.getWidth() == 0 || src.getHeight() == 0)
  {
#ifndef VDATA_NO_QT
    mutex.unlock();
#endif
    return false;
  }

  target.deepCopyFromRawImage(src, true);

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return true;
}

RawImage CaptureSplitter::getFrame()
{
#ifndef VDATA_NO_QT
   mutex.lock();
#endif

  frameMutex.lock();
  frame->deepCopyFromRawImage(*nextFrame, true);
  frameMutex.unlock();

#ifndef VDATA_NO_QT
  mutex.unlock();
#endif
  return *frame;
}

void CaptureSplitter::releaseFrame()
{
}

string CaptureSplitter::getCaptureMethodName() const
{
  return "Splitter";
}

void CaptureSplitter::onNewFrame(RawImage* frame)
{
  // TODO split image
  frameMutex.lock();
  nextFrame->deepCopyFromRawImage(*frame, true);
  frameMutex.unlock();
}
