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

static void thresholdImage(RawImage *imagePartIn, Image<raw8> *imagePartOut, YUVLUT * lut, const ImageInterface* mask = nullptr) {
  if (imagePartIn->getColorFormat() == COLOR_YUV422_UYVY) {
    CMVisionThreshold::thresholdImageYUV422_UYVY(imagePartOut, imagePartIn, lut, mask);
  } else if (imagePartIn->getColorFormat() == COLOR_YUV444) {
    CMVisionThreshold::thresholdImageYUV444(imagePartOut, imagePartIn, lut, mask);
  } else if (imagePartIn->getColorFormat() == COLOR_RGB8) {
    auto *rgblut = (RGBLUT *) lut->getDerivedLUT(CSPACE_RGB);
    if (rgblut == nullptr) {
      printf("WARNING: No RGB LUT has been defined. You need to create a derived RGB LUT by calling e.g. \"lut_yuv->addDerivedLUT(new RGBLUT(5,5,5,\"\"))\" in the stack constructor!\n");
    } else {
      CMVisionThreshold::thresholdImageRGB(imagePartOut, imagePartIn, rgblut, mask);
    }
  } else {
    fprintf(stderr, "ColorThresholding needs YUV422, YUV444, or RGB8 as input image, but found: %s\n",
            Colors::colorFormatToString(imagePartIn->getColorFormat()).c_str());
  }
}

PluginColorThresholdWorker::PluginColorThresholdWorker(int _id, int _totalThreads, YUVLUT * _lut) : QObject() {

  this->id = _id;
  this->totalThreads = _totalThreads;
  this->lut = _lut;

  thread = new QThread();
  thread->setObjectName("ColorThreshold");
  moveToThread(thread);
  connect(this, SIGNAL(startThresholding()), this, SLOT(process()));
  thread->start();
}

PluginColorThresholdWorker::~PluginColorThresholdWorker() {
  thread->quit();
  thread->deleteLater();
}


void PluginColorThresholdWorker::process() {
  RawImage imagePartIn;
  imagePartIn.setColorFormat(imageIn->getColorFormat());
  imagePartIn.setHeight(imageIn->getHeight()/totalThreads);
  imagePartIn.setWidth(imageIn->getWidth());
  int offsetBytesIn = id * imageIn->getNumBytes() / totalThreads;
  imagePartIn.setData(imageIn->getData() + offsetBytesIn);

  RawImage maskImagePartIn;
  maskImagePartIn.setColorFormat(maskImageIn->getColorFormat());
  maskImagePartIn.setHeight(maskImageIn->getHeight()/totalThreads);
  maskImagePartIn.setWidth(maskImageIn->getWidth());
  int maskOffsetBytesIn = id * maskImageIn->getNumBytes() /  totalThreads;
  maskImagePartIn.setData(maskImageIn->getData() + maskOffsetBytesIn);

  RawImage rawImageOut;
  rawImageOut.setColorFormat(imageOut->getColorFormat());
  rawImageOut.setHeight(imageOut->getHeight()/totalThreads);
  rawImageOut.setWidth(imageOut->getWidth());
  int offsetBytesOut = id * imageOut->getNumBytes() / totalThreads;
  rawImageOut.setData(imageOut->getData() + offsetBytesOut);
  Image<raw8> imagePartOut;
  imagePartOut.fromRawImage(rawImageOut);

  thresholdImage(&imagePartIn, &imagePartOut, lut, &maskImagePartIn);

  doneMutex.unlock();
}


void PluginColorThresholdWorker::start() {
  doneMutex.lock();
  emit startThresholding();
}


void PluginColorThresholdWorker::wait() {
  doneMutex.lock();
  doneMutex.unlock();
}


PluginColorThreshold::PluginColorThreshold(FrameBuffer * _buffer, YUVLUT * _lut, ConvexHullImageMask &mask)
  : VisionPlugin(_buffer), _image_mask(mask)
{
  lut=_lut;

  settings=new VarList("Color Threshold");
  numThreads = new VarInt("number of threads", 0, 0, 32);
  settings->addChild(numThreads);
}


PluginColorThreshold::~PluginColorThreshold()
{
  clearWorkers();
  delete settings;
}

void PluginColorThreshold::clearWorkers() {
  for (auto worker : workers) {
    worker->deleteLater();
  }
  workers.clear();
}


ProcessResult PluginColorThreshold::process(FrameData * data, RenderOptions * options) {
  _image_mask.lock();
  (void)options;

  Image<raw8> * img_thresholded;

  if ((img_thresholded=(Image<raw8> *)data->map.get("cmv_threshold")) == nullptr) {
    img_thresholded=(Image<raw8> *)data->map.insert("cmv_threshold",new Image<raw8>());
  }

  //make sure image is allocated:
  img_thresholded->allocate(data->video.getWidth(),data->video.getHeight());

  if((int) workers.size() != numThreads->getInt()) {
    clearWorkers();
    for(int i=0;i<numThreads->getInt();i++) {
      auto *worker = new PluginColorThresholdWorker(i, numThreads->getInt(), lut);
      workers.push_back(worker);
    }
  }

  if(workers.empty()) {
    thresholdImage(&data->video, img_thresholded, lut, &_image_mask.getMask());
  } else {
    for (auto worker : workers) {
      worker->imageIn = &data->video;
      worker->maskImageIn = &_image_mask.getMask();
      worker->imageOut = img_thresholded;
      worker->start();
    }

    for (auto worker : workers) {
      worker->wait();
    }
  }

  _image_mask.unlock();
  return ProcessingOk;
}

VarList * PluginColorThreshold::getSettings() {
  return settings;
}

string PluginColorThreshold::getName() {
  return "Segmentation";
}
