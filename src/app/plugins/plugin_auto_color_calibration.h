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
  \file    plugin_auto_color_calibration.h
  \brief   C++ Implementation: PluginAutoColorCalibration
  \author  Nicolai Ommer <nicolai.ommer@gmail.com>, (C) 2016
           Mark Geiger <markgeiger@posteo.de>, (C) 2017
*/
//========================================================================

#ifndef SRC_APP_PLUGINS_PLUGIN_ONLINE_COLOR_CALIB_H_
#define SRC_APP_PLUGINS_PLUGIN_ONLINE_COLOR_CALIB_H_

#include <visionplugin.h>
#include "cmvision_region.h"
#include "lut3d.h"
#include "field.h"
#include "camera_calibration.h"
#include "plugin_visualize.h"
#include "VarTypes.h"
#include "messages_robocup_ssl_detection.pb.h"

#include <chrono>

#include <QThread>
#include <QObject>
#include <QString>

#include <mutex>
#include <condition_variable>

#include <gui/automatedcolorcalibwidget.h>
#include <gui/lutwidget.h>

#include "initial_color_calibrator.h"


class PluginAutoColorCalibration : public VisionPlugin {
Q_OBJECT

public:
    PluginAutoColorCalibration(FrameBuffer *_buffer, YUVLUT *lut, LUTWidget *lutw);

    ~PluginAutoColorCalibration() override;

    ProcessResult process(FrameData *data, RenderOptions *options) override;

    QWidget *getControlWidget() override;

    VarList *getSettings() override;

    string getName() override;

    void mouseEvent(QMouseEvent *event, pixelloc loc);

    void mousePressEvent(QMouseEvent *event, pixelloc loc) override;

    void mouseReleaseEvent(QMouseEvent *event, pixelloc loc) override;

    void mouseMoveEvent(QMouseEvent *event, pixelloc loc) override;

private:
    void addCalibrationPoint(const yuv &color, int channel);

    void clearCalibrationPoints();

    InitialColorCalibrator initialColorCalibrator;
    AutomatedColorCalibWidget *accw = nullptr;
    VarList *settings;
    VarList *v_calibration_points;
    YUVLUT *global_lut;
    LUTWidget *lutw;

    VarDouble *v_maxDistance;
    VarDouble *v_maxAngle;
    VarList *v_weights;
    std::map<int, VarDouble *> weightMap;

    void process_gui_commands();

    float getWeight(int channel);

    VarDouble *createWeight(const std::string &colorName, int channel);

    void removeMarkedCalibrationPoints();

    void runCalibration();
};

#endif /* SRC_APP_PLUGINS_PLUGIN_ONLINE_COLOR_CALIB_H_ */
