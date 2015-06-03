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
  \file    field_default_constants.h
  \brief   Definition of field dimensions
  \author  Joydeep Biswas, (C) 2013
*/
//========================================================================

#include "field_default_constants.h"

#include <math.h>
#include "field.h"

namespace FieldConstantsRoboCup2014 {

const std::size_t kNumFieldLines = 8;
const FieldLine kFieldLines[kNumFieldLines] = {
  FieldLine("TopTouchLine", -4490, 2995, 4490, 2995, 10),
  FieldLine("BottomTouchLine", -4490, -2995, 4490, -2995, 10),
  FieldLine("LeftGoalLine", -4490, -2995, -4490, 2995, 10),
  FieldLine("RightGoalLine", 4490, -2995, 4490, 2995, 10),
  FieldLine("HalfwayLine", 0, -2995, 0, 2995, 10),
  FieldLine("CenterLine", -4490, 0, 4490, 0, 10),
  FieldLine("LeftPenaltyStretch", -3500, -250, -3500, 250, 10),
  FieldLine("RightPenaltyStretch", 3500, -250, 3500, 250, 10),
};

const std::size_t kNumFieldArcs = 5;
const FieldCircularArc kFieldArcs[kNumFieldArcs] = {
  FieldCircularArc("LeftFieldLeftPenaltyArc",
                   -4495, 250, 995, 0, 0.5 * M_PI, 10),
  FieldCircularArc("LeftFieldRightPenaltyArc",
                   -4495, -250, 995, 1.5 * M_PI, 2.0 * M_PI, 10),
  FieldCircularArc("RightFieldLeftPenaltyArc",
                   4495, -250, 995, M_PI, 1.5 * M_PI, 10),
  FieldCircularArc("RightFieldRightPenaltyArc",
                   4495, 250, 995, 0.5 * M_PI, M_PI, 10),
  FieldCircularArc("CenterCircle",
                   0, 0, 495, 0, 2.0 * M_PI, 10),
};

const GVector::vector2d<double> kCameraControlPoints[4][4] = {
  {
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(0.0, 4495.0),
    GVector::vector2d<double>(4495.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
  },

  {
    GVector::vector2d<double>(-4495.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(0.0, 3025.0),
    GVector::vector2d<double>(-4495.0, 3025.0),
  },

  {
    GVector::vector2d<double>(-4495.0, -3025.0),
    GVector::vector2d<double>(0.0, -3025.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(-4495.0, 0.0),
  },

  {
    GVector::vector2d<double>(0.0, -3025.0),
    GVector::vector2d<double>(4495.0, -3025.0),
    GVector::vector2d<double>(4495.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
  }
};

}  // namespace FieldConstantsRoboCup2012
