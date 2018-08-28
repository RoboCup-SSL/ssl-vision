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

#define LUT_COPY_PLACEHOLDER    "(click copy to refresh)"

PluginColorCalibration::PluginColorCalibration(FrameBuffer * _buffer, YUVLUT * _lut, LUTChannelMode _mode) : VisionPlugin(_buffer)
{
    mode=_mode;
    lut=_lut;
    lutw=NULL;
    settings=new VarList("YUV Calibrator");
    continuing_undo = false;
    
    copy_LUT_trigger = new VarTrigger("Copy LUT", "Copy LUT");
    settings->addChild(copy_LUT_trigger);
    connect(copy_LUT_trigger,SIGNAL(signalTriggered()),this,SLOT(slotCopyLUT()));
    settings->addChild( v_lut_sources = new VarStringEnum("LUT Source", LUT_COPY_PLACEHOLDER));
    v_lut_sources->addItem(LUT_COPY_PLACEHOLDER);
    settings->addChild( v_lut_colors = new VarStringEnum("Color", LUT_COPY_PLACEHOLDER));
    updateColorList();
}

void PluginColorCalibration::updateColorList() {
    v_lut_colors->setSize(0);
    v_lut_colors->addItem("all");
    for (std::vector<LUTChannel>::iterator it = lut->channels.begin(); it != lut->channels.end(); it++) {
        // the '<>' chars can not be stored in VarTypes, so remove it here
        std::string label = it->label;
        if(label == "<Clear>") label = "Clear";
        v_lut_colors->addItem(label);
    }
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
    delete copy_LUT_trigger;
    delete v_lut_sources;
    delete v_lut_colors;
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
              if(event->modifiers() & Qt::ShiftModifier) {          //modified 2/1/16, 'control' alone consumed
                if(event->modifiers() & Qt::ControlModifier)        // by QT framework to move a window
                  lutw->add_del_Pixel(color, true, continuing_undo);
                else
                  lutw->add_del_Pixel(color, false, continuing_undo);
              }
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

void PluginColorCalibration::slotCopyLUT() {
    copyLUT();
}

void PluginColorCalibration::copyLUT() {
    updateColorList();
    VarString *pSelf = reinterpret_cast<VarString*>(settings->getParent());
    if (v_lut_sources->getCount()==1 && v_lut_sources->getString()==LUT_COPY_PLACEHOLDER) {
        std::vector<VarType *> vectRelatives = settings->findRelatives("LUT 3D", true);
        if (vectRelatives.size() < 1) { // nothing like LUTs? abort
            fprintf(stderr, "LUT3D: Could not find any LUT3D nodes for sources, aborting copy.\n");
        }
        else {
            //truncate/clear former list
            v_lut_sources->setSize(0);
            for (uint iCam=0; iCam<vectRelatives.size(); iCam++) {
                VarString *pCamera = reinterpret_cast<VarString*>(vectRelatives[iCam]->getParent()->getParent());
                if (pCamera != pSelf)
                    v_lut_sources->addItem(pCamera->getName());
            }
            if (!vectRelatives.empty()) {
                v_lut_sources->selectIndex(0);
            }
        }
    }
    else { //user has selected a camera
        VarBlob *pSource = NULL;
        int color_index = v_lut_colors->getIndex() - 1;
        std::vector<VarType *> vectTarget = settings->findRelatives(v_lut_sources->getString(), true);
        if (vectTarget.size()!=1) {
            fprintf(stderr, "LUT3D: Found too many camera nodes (%d) for '%s', aborting copy.\n",
                    static_cast<int>(vectTarget.size()), v_lut_sources->getString().c_str());
            return;
        }
        vectTarget = vectTarget[0]->findRelatives("LUT Data", false);  //just children
        if (vectTarget.size()!=1) {
            fprintf(stderr, "LUT3D: Found too many LUT nodes (%d) for camera '%s', aborting copy.\n",
                    static_cast<int>(vectTarget.size()), v_lut_sources->getString().c_str());
            return;
        }

        pSource = reinterpret_cast<VarBlob*>(vectTarget[0]);

        //WARNING: Makes assumption that all LUT blobs have the same properties
        //  (e.g. same initializer, etc) because we do not reallocate, modify shift
        //  indicies, or anything else dramatic.  This is an acceptable risk because
        //  the constructor for this class just loops to add multiple camera sources.
        //  (added corresponding warning in multistack_robocup_ssl.cpp at allocation)

        GLLUTWidget *pGLLUTw = lutw->getGLLUTWidget();
        if (pGLLUTw->copyLUT(pSource->getDataPointer(), (int) pSource->getDataSize(), color_index)) {
            fprintf(stderr, "PluginColorCalibration: Successfully copied LUT data for color '%s' from '%s' to '%s'.\n",
                    v_lut_colors->getString().c_str(), v_lut_sources->getString().c_str(), pSelf->getName().c_str());
        }
    }
}

