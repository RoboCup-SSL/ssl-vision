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
  \file    realtimedisplaywidget.h
  \brief   C++ Interface: RealTimeDisplayWidget
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef RTDISPLAYWIDGET_H_
#define RTDISPLAYWIDGET_H_
#include "framedata.h"
#include "ringbuffer.h"
#include "videostats.h"
#include "renderoptions.h"

/*!
  \class   RealTimeDisplayWidget
  \brief   A base-class for all visualization-related display-widgets
  \author  Stefan Zickler, (C) 2008
*/
class RealTimeDisplayWidget {
  protected:
    VideoStats stats;
    FrameBuffer * rb;

  public:
    RealTimeDisplayWidget(FrameBuffer * _rb = 0);
    virtual ~RealTimeDisplayWidget();

    FrameBuffer * getRingBuffer();
    void setRingBuffer(FrameBuffer * _rb);

    //this function will be called about 1000 times/s on your visualization plugin
    //in most cases you might want to only trigger a render if frame_changed==true
    //which should occur with the same frequency as your camera input
    virtual void displayLoopEvent(bool frame_changed, RenderOptions * opts);

};

#endif /*RTDISPLAYWIDGET_H_*/
