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
  \file    plugin_colorthreshold.h
  \brief   C++ Interface: plugin_colorthreshold
  \author  Author Name, 2008
*/
//========================================================================
#ifndef PLUGIN_COLORTHRESHOLD_H
#define PLUGIN_COLORTHRESHOLD_H

#include <visionplugin.h>
#include "lut3d.h"
#include "cmvision_threshold.h"
#include <mutex>
#include <QThread>
#include <QObject>
#include "convex_hull_image_mask.h"

class PluginColorThresholdWorker : public QObject {
Q_OBJECT
public:
    PluginColorThresholdWorker(int id, int totalThreads, YUVLUT * lut);
    ~PluginColorThresholdWorker() override;
    QThread* thread;
    int id;
    int totalThreads;
    RawImage* imageIn = nullptr;
    const ImageInterface* maskImageIn = nullptr;
    Image<raw8>* imageOut = nullptr;
    YUVLUT * lut;
    std::mutex doneMutex;

    void start();
    void wait();

public slots:
    void process();

signals:
    void startThresholding();

};

/**
	@author Stefan Zickler
*/
class PluginColorThreshold : public VisionPlugin
{
protected:
  YUVLUT * lut;
  ConvexHullImageMask& _image_mask;
  VarList * settings;
  VarInt * numThreads;
public:
  PluginColorThreshold(FrameBuffer * _buffer, YUVLUT * _lut, ConvexHullImageMask& mask);

    ~PluginColorThreshold() override;

    ProcessResult process(FrameData * data, RenderOptions * options) override;

    VarList * getSettings() override;

    string getName() override;
private:
    std::vector<PluginColorThresholdWorker*> workers;

    void clearWorkers();
};

#endif
