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


ColorClazz::ColorClazz(
        const yuv &initColor,
        int clazz,
        float weight,
        float maxDistance,
        float maxAngle)
        : color_yuv(initColor),
        clazz(clazz),
        weight(weight),
        maxDistance(maxDistance),
        maxAngle(maxAngle) {
}

float InitialColorCalibrator::ratedYuvColorDist(const yuv &c1, const ColorClazz &colorClazz) {
  yuv c2 = colorClazz.color_yuv;
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
  float relAngle = angle / colorClazz.maxAngle;
  float dMaxHalf = colorClazz.maxDistance * 0.5f;

  float bonus;
  if (relAngle < 1) {
    // give bonus for good angles
    bonus = (1 - relAngle) * dMaxHalf;
  } else {
    // give penalty for bad angles
    bonus = -relAngle * dMaxHalf;
  }

  float y = c1.y - c2.y;
  float u = c1.u - c2.u;
  float v = c1.v - c2.v;

  float yuvDist = y * y + u * u + v * v;
  return (yuvDist - bonus) * colorClazz.weight;
}

void InitialColorCalibrator::process(const std::vector<ColorClazz> &calibration_points, YUVLUT *global_lut) {
  global_lut->lock();
  for (int y = 0; y <= 255; y += (0x1u << global_lut->X_SHIFT)) {
    for (int u = 0; u <= 255; u += (0x1u << global_lut->Y_SHIFT)) {
      for (int v = 0; v <= 255; v += (0x1u << global_lut->Z_SHIFT)) {
        yuv color = yuv(static_cast<unsigned char>(y),
                        static_cast<unsigned char>(u),
                        static_cast<unsigned char>(v));
        float minScore = 1e10;
        int clazz = 0;
        float maxDistance = 0;
        for (auto &colorClazz : calibration_points) {
          float score = ratedYuvColorDist(color, colorClazz);
          if (score < minScore) {
            minScore = score;
            clazz = colorClazz.clazz;
            maxDistance = colorClazz.maxDistance;
          }
        }

        if (minScore < maxDistance) {
          global_lut->set(color.y, color.u, color.v, static_cast<lut_mask_t>(clazz));
        }
      }
    }
  }
  global_lut->unlock();
  global_lut->updateDerivedLUTs();
}
