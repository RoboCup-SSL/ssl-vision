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
  \file    initial_color_calibrator.h
  \brief   C++ Interface: InitialColorCalibrator
  \author  Mark Geiger <MarkGeiger@posteo.de>, (C) 2017
*/
//========================================================================
#ifndef INITIAL_COLOR_CALIBRATOR_H
#define INITIAL_COLOR_CALIBRATOR_H

#include "colors.h"

class ColorClazz {
public:
    ColorClazz(
            const yuv &initColor,
            int clazz,
            float weight,
            float maxDistance,
            float maxAngle);

    yuv color_yuv;
    int clazz;
    float weight;
    float maxDistance;
    float maxAngle;
};

class InitialColorCalibrator {

public:
    InitialColorCalibrator() = default;

    ~InitialColorCalibrator() = default;

    void process(const std::vector<ColorClazz> &calibration_points, YUVLUT *global_lut);

private:
    float ratedYuvColorDist(const yuv &c1, const ColorClazz &colorClazz);
};


#endif //INITIAL_COLOR_CALIBRATOR_H
