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
  \file    plugin_colorthreshold.cpp
  \brief   C++ Implementation: plugin_colorthreshold
  \author  Stefan Zickler, 2008
*/
//========================================================================
#include "plugin_colorthreshold.h"

PluginColorThreshold::PluginColorThreshold(FrameBuffer * _buffer, YUVLUT * _lut)
 : VisionPlugin(_buffer)
{
  lut=_lut;

  int n=4;
  for(int i=0;i<n;i++) {
    auto *thread = new QThread();
    thread->setObjectName("ColorThresholding");
    moveToThread(thread);
    auto *worker = new PluginColorThresholdWorker(thread, i, n, lut);
    workers.push_back(worker);
    connect(thread, SIGNAL(started()), worker, SLOT(thresholdImage()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  }
}


PluginColorThreshold::~PluginColorThreshold()
{
  for(auto worker : workers) {
    delete worker->thread;
    delete worker;
  }
  workers.clear();
}


PluginColorThresholdWorker::PluginColorThresholdWorker(QThread* thread, int id, int totalThreads, YUVLUT * lut)
: thread(thread), id(id), totalThreads(totalThreads), lut(lut) {
}


void PluginColorThresholdWorker::thresholdImage() {
  RawImage imagePartIn;
  imagePartIn.setColorFormat(imageIn->getColorFormat());
  imagePartIn.setHeight(imageIn->getHeight()/totalThreads);
  imagePartIn.setWidth(imageIn->getWidth());
  int offsetBytesIn = id * imageIn->getNumBytes() / totalThreads;
  imagePartIn.setData(imageIn->getData() + offsetBytesIn);

  RawImage rawImageOut;
  rawImageOut.setColorFormat(imageOut->getColorFormat());
  rawImageOut.setHeight(imageOut->getHeight()/totalThreads);
  rawImageOut.setWidth(imageOut->getWidth());
  int offsetBytesOut = id * imageOut->getNumBytes() / totalThreads;
  rawImageOut.setData(imageOut->getData() + offsetBytesOut);
  Image<raw8> imagePartOut;
  imagePartOut.fromRawImage(rawImageOut);

  if (imageIn->getColorFormat()==COLOR_YUV422_UYVY) {
    //directly apply YUV lut:
    CMVisionThreshold::thresholdImageYUV422_UYVY(&imagePartOut,&imagePartIn,lut);
  } else if (imageIn->getColorFormat()==COLOR_YUV444) {
    //directly apply YUV lut:
    CMVisionThreshold::thresholdImageYUV444(&imagePartOut,&imagePartIn,lut);
  } else if (imageIn->getColorFormat()==COLOR_RGB8) {
    auto * rgblut = (RGBLUT *) lut->getDerivedLUT(CSPACE_RGB);
    if (rgblut == nullptr) {
      printf("WARNING: No RGB LUT has been defined. You need to create a derived RGB LUT by calling e.g. \"lut_yuv->addDerivedLUT(new RGBLUT(5,5,5,\"\"))\" in the stack constructor!\n");
    } else {
      CMVisionThreshold::thresholdImageRGB(&imagePartOut,&imagePartIn,rgblut);
    }
  } else {
    fprintf(stderr,"ColorThresholding needs YUV422, YUV444, or RGB8 as input image, but found: %s\n",Colors::colorFormatToString(imageIn->getColorFormat()).c_str());
  }
}


ProcessResult PluginColorThreshold::process(FrameData * data, RenderOptions * options) {
  (void)options;
  
  Image<raw8> * img_thresholded;

  if ((img_thresholded=(Image<raw8> *)data->map.get("cmv_threshold")) == nullptr) {
    img_thresholded=(Image<raw8> *)data->map.insert("cmv_threshold",new Image<raw8>());
  }

  //make sure image is allocated:
  img_thresholded->allocate(data->video.getWidth(),data->video.getHeight());

  for(auto worker : workers) {
    worker->imageIn = &data->video;
    worker->imageOut = img_thresholded;
    worker->thread->start();
  }

  for(auto worker : workers) {
    worker->thread->wait();
  }
  
  return ProcessingOk;
}

VarList * PluginColorThreshold::getSettings() {
  return nullptr;
}

string PluginColorThreshold::getName() {
  return "Segmentation";
}
