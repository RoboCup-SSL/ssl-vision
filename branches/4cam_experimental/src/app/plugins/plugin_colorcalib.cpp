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
  \file    plugin_colorcalib.cpp
  \brief   C++ Implementation: plugin_colorcalib
  \author  Author Name, 2008
*/
//========================================================================
#include "plugin_colorcalib.h"
#include <QStackedWidget>

PluginColorCalibration::PluginColorCalibration(FrameBuffer * _buffer, YUVLUT * _lut, LUTChannelMode _mode) : VisionPlugin(_buffer)
{
  mode=_mode;
  lut=_lut;
  lutw=0;
  settings=new VarList("YUV Calibrator");
  continuing_undo = false;
}

VarList * PluginColorCalibration::getSettings() {
  return settings;
}

string PluginColorCalibration::getName() {
  return "YUV Calibration";
}

PluginColorCalibration::~PluginColorCalibration()
{
  delete settings;
}

QWidget * PluginColorCalibration::getControlWidget() {
  if (lutw==0) {
    lutw=new LUTWidget(lut,mode);
    connect(lutw->getGLLUTWidget(),SIGNAL(signalKeyPressEvent(QKeyEvent *)),this, SLOT(slotKeyPressEvent(QKeyEvent *)));
  }
  return (QWidget *)lutw;
}

void PluginColorCalibration::mouseEvent( QMouseEvent * event, pixelloc loc) {
  QTabWidget* tabw = (QTabWidget*) lutw->parentWidget()->parentWidget();  
  if (tabw->currentWidget() == lutw) {
    if (event->buttons()==Qt::LeftButton) {
      FrameBuffer * rb=getFrameBuffer();
      if (rb!=0) {
        rb->lockRead();
        int idx=rb->curRead();
        FrameData * frame = rb->getPointer(idx);
        if (loc.x < frame->video.getWidth() && loc.y < frame->video.getHeight() && loc.x >=0 && loc.y >=0) {
          if (frame->video.getWidth() > 1 && frame->video.getHeight() > 1) {
            yuv color;
            //if converting entire image then blanking is not needed
            ColorFormat source_format=frame->video.getColorFormat();
            if (source_format==COLOR_RGB8) {
              //plain copy of data
                rgbImage img(frame->video);
              color=Conversions::rgb2yuv(img.getPixel(loc.x,loc.y));
            } else if (source_format==COLOR_YUV444) {
              yuvImage img(frame->video);
              color=img.getPixel(loc.x,loc.y);
            } else if (source_format==COLOR_YUV422_UYVY) {
              uyvy color2 = *((uyvy*)(frame->video.getData() + (sizeof(uyvy) * (((loc.y * (frame->video.getWidth())) + loc.x) / 2))));
              color.u=color2.u;
              color.v=color2.v;
              if ((loc.x % 2)==0) {
                color.y=color2.y1;
              } else {
                color.y=color2.y2;
              }
            } else {
              //blank it:
              fprintf(stderr,"Unable to pick color from frame of format: %s\n",Colors::colorFormatToString(source_format).c_str());
              fprintf(stderr,"Currently supported are rgb8, yuv444, and yuv422 (UYVY).\n");
              fprintf(stderr,"(Feel free to add more conversions to plugin_colorcalib.cpp).\n");
            }
            lutw->samplePixel(color);
            //img.setPixel(loc.x,loc.y,rgb(255,0,0));

            if (event->modifiers()!=Qt::NoModifier) {
              if(event->modifiers() & Qt::ShiftModifier) 
                lutw->add_del_Pixel(color, false, continuing_undo);
              else if(event->modifiers() & Qt::ControlModifier) 
                lutw->add_del_Pixel(color, true, continuing_undo);
              continuing_undo = true;
            }
          }
        }
        rb->unlockRead();
      }
      event->accept();
    }
  
  }
  else
    event->ignore();
}

void PluginColorCalibration::keyPressEvent ( QKeyEvent * event ) {
  if (event->key()==Qt::Key_I) {
    FrameBuffer * rb=getFrameBuffer();
    if (rb!=0) {
      rb->lockRead();
      int idx=rb->curRead();
      FrameData * frame = rb->getPointer(idx);
      lutw->sampleImage(frame->video);
      rb->unlockRead();
    }
    event->accept();
  } else if (event->key()==Qt::Key_C) {
    lutw->getGLLUTWidget()->clearSampler();
    event->accept();
  }
}

void PluginColorCalibration::mousePressEvent ( QMouseEvent * event, pixelloc loc ) {
  mouseEvent(event,loc);
}

void PluginColorCalibration::mouseReleaseEvent ( QMouseEvent * event, pixelloc loc ) {
  continuing_undo = false;
  mouseEvent(event,loc);
}

void PluginColorCalibration::mouseMoveEvent ( QMouseEvent * event, pixelloc loc ) {
  mouseEvent(event,loc);
}
