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

// Note(Joydeep): This #define is used to select between the double-sized 4-
// camera field, and the regular 2013 2-camera field to test the new calibration
// software on the old field.
#define DOUBLE_SIZED_FIELD

namespace FieldConstantsRoboCup2014 {

#ifdef DOUBLE_SIZED_FIELD
const std::size_t kNumFieldLines = 8;
const FieldLine kFieldLines[kNumFieldLines] = {
  FieldLine("TopTouchLine", -4500, 2995, 4500, 2995, 10),
  FieldLine("BottomTouchLine", -4500, -2995, 4500, -2995, 10),
  FieldLine("LeftGoalLine", -4495, -2995, -4495, 2995, 10),
  FieldLine("RightGoalLine", 4495, -2995, 4495, 2995, 10),
  FieldLine("HalfwayLine", 0, -2995, 0, 2995, 10),
  FieldLine("CenterLine", -4500, 0, 4500, 0, 10),
  FieldLine("LeftPenaltyStretch", -3505, -500, -3505, 500, 10),
  FieldLine("RightPenaltyStretch", 3505, -500, 3505, 500, 10),
};

const std::size_t kNumFieldArcs = 5;
const FieldCircularArc kFieldArcs[kNumFieldArcs] = {
  FieldCircularArc("LeftFieldLeftPenaltyArc",
                   -4500, 500, 995, 0, 0.5 * M_PI, 10),
  FieldCircularArc("LeftFieldRightPenaltyArc",
                   -4500, -500, 995, 1.5 * M_PI, 2.0 * M_PI, 10),
  FieldCircularArc("RightFieldLeftPenaltyArc",
                   4500, -500, 995, M_PI, 1.5 * M_PI, 10),
  FieldCircularArc("RightFieldRightPenaltyArc",
                   4500, 500, 995, 0.5 * M_PI, M_PI, 10),
  FieldCircularArc("CenterCircle",
                   0, 0, 745, 0, 2.0 * M_PI, 10),
};

const GVector::vector2d<double> kCameraControlPoints[4][4] = {
  {
    GVector::vector2d<double>(0.0, -3000.0),
    GVector::vector2d<double>(4500.0, -3000.0),
    GVector::vector2d<double>(4500.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
  },

  {
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(4500.0, 0.0),
    GVector::vector2d<double>(4500.0, 3000.0),
    GVector::vector2d<double>(0.0, 3000.0),
  },

  {
    GVector::vector2d<double>(-4500.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(0.0, 3000.0),
    GVector::vector2d<double>(-4500.0, 3000.0),
  },

  {
    GVector::vector2d<double>(-4500.0, -3000.0),
    GVector::vector2d<double>(0.0, -3000.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(-4500.0, 0.0),
  }
};

#else
const std::size_t kNumFieldLines = 7;
const FieldLine kFieldLines[kNumFieldLines] = {
  FieldLine("TopTouchLine", -3025, 2020, 3025, 2020, 10),
  FieldLine("BottomTouchLine", -3025, -2020, 3025, -2020, 10),
  FieldLine("LeftGoalLine", -3020, -2025, -3020, 2025, 10),
  FieldLine("RightGoalLine", 3020, -2025, 3020, 2025, 10),
  FieldLine("HalfwayLine", 0, -2025, 0, 2025, 10),
  FieldLine("LeftPenaltyStretch", -2225, -175, -2225, 175, 10),
  FieldLine("RightPenaltyStretch", 2225, -175, 2225, 175, 10),
};

const std::size_t kNumFieldArcs = 5;
const FieldCircularArc kFieldArcs[kNumFieldArcs] = {
  FieldCircularArc("LeftFieldLeftPenaltyArc",
                   -3025, 175, 795, 0, 0.5 * M_PI, 10),
  FieldCircularArc("LeftFieldRightPenaltyArc",
                   -3025, -175, 795, 1.5 * M_PI, 2.0 * M_PI, 10),
  FieldCircularArc("RightFieldLeftPenaltyArc",
                   3025, -175, 795, M_PI, 1.5 * M_PI, 10),
  FieldCircularArc("RightFieldRightPenaltyArc",
                   3025, 175, 795, 0.5 * M_PI, M_PI, 10),
  FieldCircularArc("CenterCircle",
                   0, 0, 500, 0, 2.0 * M_PI, 10),
};

const GVector::vector2d<double> kCameraControlPoints[4][4] = {
  {
    GVector::vector2d<double>(0.0, -2025.0),
    GVector::vector2d<double>(3020.0, -2025.0),
    GVector::vector2d<double>(3020.0, 2025.0),
    GVector::vector2d<double>(0.0, 2025.0),
  },

  {
    GVector::vector2d<double>(0.0, 2025.0),
    GVector::vector2d<double>(-3020.0, 2025.0),
    GVector::vector2d<double>(-3020.0, -2025.0),
    GVector::vector2d<double>(0.0, -2025.0),
  },

  {
    GVector::vector2d<double>(-3020.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(0.0, 2025.0),
    GVector::vector2d<double>(-3020.0, 2025.0),
  },

  {
    GVector::vector2d<double>(-3020.0, -2025.0),
    GVector::vector2d<double>(0.0, -2025.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(-3020.0, 0.0),
  }
};

#endif

}  // namespace FieldConstantsRoboCup2012
