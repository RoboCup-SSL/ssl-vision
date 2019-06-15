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
  \file    initial_color_calibrator.cpp
  \brief   C++ Implementation: InitialColorCalibrator
  \author  Mark Geiger <MarkGeiger@posteo.de>, (C) 2017
*/
//========================================================================
#include <framedata.h>
#include <plugins/visionplugin.h>
#include "conversions.h"
#include "image.h"
#include "camera_calibration.h"
#include "lut3d.h"
#include "initial_color_calibrator.h"

#define CH_ORANGE 2
#define CH_YELLOW 3
#define CH_BLUE 4
#define CH_PINK 5
#define CH_GREEN 7

VarDouble* InitialColorCalibrator::createWeight(const std::string &colorName, int channel) {
  VarDouble *v_weight = new VarDouble(colorName, 1, 0, 10);
  weightMap[channel] = v_weight;
  return v_weight;
}

InitialColorCalibrator::InitialColorCalibrator() {
  maxColorDist = new VarDouble("Max color distance", 3000, 0);
  maxAngle = new VarDouble("Max angle", 0.3, 0, 3.14);

  weights = new VarList("Weights");
  weights->addChild(createWeight("Orange", CH_ORANGE));
  weights->addChild(createWeight("Yellow", CH_YELLOW));
  weights->addChild(createWeight("Blue", CH_BLUE));
  weights->addChild(createWeight("Pink", CH_PINK));
  weights->addChild(createWeight("Green", CH_GREEN));
}

ColorClazz::ColorClazz(const yuv &initColor, int clazz)
        : color_yuv(initColor), clazz(clazz) {
}

double InitialColorCalibrator::ratedYuvColorDist(const yuv &c1, const yuv &c2, const double weight) {
  float midToC1U = c1.u - 127;
  float midToC2U = c2.u - 127;
  float midToC1V = c1.v - 127;
  float midToC2V = c2.v - 127;
  float normFac1 = std::sqrt(midToC1U * midToC1U + midToC1V * midToC1V);
  float normFac2 = std::sqrt(midToC2U * midToC2U + midToC2V * midToC2V);
  midToC1U *= 1 / normFac1;
  midToC1V *= 1 / normFac1;
  midToC2U *= 1 / normFac2;
  midToC2V *= 1 / normFac2;
  float scalar = midToC1U * midToC2U + midToC1V * midToC2V;
  float angle = std::acos(scalar);

  float bonus = 0;
  if (angle < maxAngle->getDouble()) {
    // give bonus for good angles
    bonus = (1 - (angle / maxAngle->getDouble())) * maxColorDist->getDouble() * 0.5f;
  } else {
    // give penalty for bad angles
    angle = std::min(1.57f, angle);
    bonus = -maxColorDist->getDouble() * 0.5f * angle / maxAngle->getDouble();
  }

  float u = c1.u - c2.u;
  float v = c1.v - c2.v;
  float y = c1.y - c2.y;

  float uvDist = u * u + v * v;
  return ((uvDist + y * y) - bonus) * weight;
}

void InitialColorCalibrator::process(const std::vector<ColorClazz> &calibration_points, YUVLUT *global_lut) {
  global_lut->lock();
  for (int y = 0; y <= 255; y += (0x1u << global_lut->X_SHIFT)) {
    for (int u = 0; u <= 255; u += (0x1u << global_lut->Y_SHIFT)) {
      for (int v = 0; v <= 255; v += (0x1u << global_lut->Z_SHIFT)) {
        yuv color = yuv(static_cast<unsigned char>(y),
                        static_cast<unsigned char>(u),
                        static_cast<unsigned char>(v));
        float minDiff = 1e10;
        int clazz = 0;
        for (auto &j : calibration_points) {
          VarDouble* v_weight = weightMap[j.clazz];
          double weight = 1;
          if(v_weight != nullptr) {
            weight = v_weight->getDouble();
          }
          double diff = ratedYuvColorDist(color, j.color_yuv, weight);
          if (diff < minDiff) {
            minDiff = diff;
            clazz = j.clazz;
          }
        }

        if (minDiff < maxColorDist->getDouble()) {
          global_lut->set(color.y, color.u, color.v, static_cast<lut_mask_t>(clazz));
        }
      }
    }
  }
  global_lut->unlock();
  global_lut->updateDerivedLUTs();
}

