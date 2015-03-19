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
// #define STANDARD_DUAL_FIELD

namespace FieldConstantsRoboCup2014 {

#ifdef STANDARD_DUAL_FIELD
const std::size_t kNumFieldLines = 12;
const FieldLine kFieldLines[kNumFieldLines] = {
  FieldLine("TopTouchLine", -4040, 3020, 4040, 3020, 10),
  FieldLine("BottomTouchLine", -4040, -3020, 4040, -3020, 10),
  FieldLine("LeftGoalLine", -4040, -3020, -4040, 3020, 10),
  FieldLine("RightGoalLine", 4040, -3020, 4040, 3020, 10),
  FieldLine("HalfwayLine", 0, -3020, 0, 3020, 10),
  FieldLine("CenterLine", -4040, 0, 4040, 0, 10),
  FieldLine("LeftPenaltyStretch", -3050, -250, -3050, 250, 10),
  FieldLine("RightPenaltyStretch", 3050, -250, 3050, 250, 10),
  
  // The following are included for the overlaid single-sized fields at 
  // RoboCup 2014.
  // Primary Single-Size Field.
  FieldLine("PrimarySingleSize_LeftPenaltyStretch", 
            -2195, -2230, -1845, -2230, 10),
  FieldLine("PrimarySingleSize_RightPenaltyStretch", 
            -2195, 2230, -1845, 2230, 10),
  
  // Secondary Single-Size Field.
  FieldLine("SecondarySingleSize_LeftPenaltyStretch", 
            2195, -2230, 1845, -2230, 10),
  FieldLine("SecondarySingleSize_RightPenaltyStretch", 
            2195, 2230, 1845, 2230, 10),
};

const std::size_t kNumFieldArcs = 15;
const FieldCircularArc kFieldArcs[kNumFieldArcs] = {
  FieldCircularArc("LeftFieldLeftPenaltyArc",
                   -4045, 250, 995, 0, 0.5 * M_PI, 10),
  FieldCircularArc("LeftFieldRightPenaltyArc",
                   -4045, -250, 995, 1.5 * M_PI, 2.0 * M_PI, 10),
  FieldCircularArc("RightFieldLeftPenaltyArc",
                   4045, -250, 995, M_PI, 1.5 * M_PI, 10),
  FieldCircularArc("RightFieldRightPenaltyArc",
                   4045, 250, 995, 0.5 * M_PI, M_PI, 10),
  FieldCircularArc("CenterCircle",
                   0, 0, 495, 0, 2.0 * M_PI, 10),
  
  // The following are included for the overlaid single-sized fields at 
  // RoboCup 2014.
  // Primary Single-Size Field.
  FieldCircularArc("PrimarySingleSize_CenterCircle",
                   -2020, 0, 495, 0, 2.0 * M_PI, 10),
  FieldCircularArc("PrimarySingleSize_LeftFieldLeftPenaltyArc",
                   -2195, -3025, 795, 0.5 * M_PI, M_PI, 10),
  FieldCircularArc("PrimarySingleSize_LeftFieldRightPenaltyArc",
                   -1845, -3025, 795, 0.0, 0.5 * M_PI, 10),
  FieldCircularArc("PrimarySingleSize_RightFieldRightPenaltyArc",
                   -2195, 3025, 795, M_PI, 1.5 * M_PI, 10),
  FieldCircularArc("PrimarySingleSize_RightFieldLeftPenaltyArc",
                   -1845, 3025, 795, 1.5 * M_PI, 2.0 * M_PI, 10),

  // Secondary Single-Size Field.
  FieldCircularArc("SecondarySingleSize_CenterCircle",
                   2020, 0, 495, 0, 2.0 * M_PI, 10),
  FieldCircularArc("SecondarySingleSize_LeftFieldRightPenaltyArc",
                   2195, -3025, 795, 0.0, 0.5 * M_PI, 10),
  FieldCircularArc("SecondarySingleSize_LeftFieldLeftPenaltyArc",
                   1845, -3025, 795, 0.5 * M_PI, M_PI, 10),
  FieldCircularArc("SecondarySingleSize_RightFieldLeftPenaltyArc",
                   2195, 3025, 795, 1.5 * M_PI, 2.0 * M_PI, 10),
  FieldCircularArc("SecondarySingleSize_RightFieldRightPenaltyArc",
                   1845, 3025, 795, M_PI, 1.5 * M_PI, 10),
};

const GVector::vector2d<double> kCameraControlPoints[4][4] = {
  {
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(0.0, 4045.0),
    GVector::vector2d<double>(4045.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
  },

  {
    GVector::vector2d<double>(-4045.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(0.0, 3025.0),
    GVector::vector2d<double>(-4045.0, 3025.0),
  },

  {
    GVector::vector2d<double>(-4045.0, -3025.0),
    GVector::vector2d<double>(0.0, -3025.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(-4045.0, 0.0),
  },

  {
    GVector::vector2d<double>(0.0, -3025.0),
    GVector::vector2d<double>(4045.0, -3025.0),
    GVector::vector2d<double>(4045.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
  }
};

#else
const std::size_t kNumFieldLines = 10;
const FieldLine kFieldLines[kNumFieldLines] = {
  FieldLine("TopTouchLine", -4050, 2830, 2960, 2830, 10),
  FieldLine("BottomTouchLine", -4050, -2830, 2960, -2830, 10),
  FieldLine("LeftGoalLine", -4050, -2830, -4050, 2830, 10),
  FieldLine("RightGoalLine", 2960, -2830, 2960, 2830, 10),
  FieldLine("HalfwayLine", 0, -2830, 0, 2830, 10),
  FieldLine("CenterLine", -4050, 0, 2960, 0, 10),
  FieldLine("LeftPenaltyStretch", -3050, -250, -3050, 250, 10),
  FieldLine("RightPenaltyStretch", 1960, -250, 1960, 250, 10),
  
  // The following are included for the overlaid single-sized fields at 
  // RoboCup 2014.
  // Primary Single-Size Field.
  FieldLine("PrimarySingleSize_LeftPenaltyStretch", 
            -1850, -2030, -2200, -2030, 10),
  FieldLine("PrimarySingleSize_RightPenaltyStretch", 
            -1850, 2030, -2200, 2030, 10)
};

const std::size_t kNumFieldArcs = 10;
const FieldCircularArc kFieldArcs[kNumFieldArcs] = {
  FieldCircularArc("LeftFieldLeftPenaltyArc",
                   -4045, 250, 995, 0, 0.5 * M_PI, 10),
  FieldCircularArc("LeftFieldRightPenaltyArc",
                   -4045, -250, 995, 1.5 * M_PI, 2.0 * M_PI, 10),
  FieldCircularArc("RightFieldLeftPenaltyArc",
                   2960, -250, 995, M_PI, 1.5 * M_PI, 10),
  FieldCircularArc("RightFieldRightPenaltyArc",
                   2960, 250, 995, 0.5 * M_PI, M_PI, 10),
  FieldCircularArc("CenterCircle",
                   0, 0, 495, 0, 2.0 * M_PI, 10),
  
  // The following are included for the overlaid single-sized fields at 
  // RoboCup 2014.
  // Primary Single-Size Field.
  FieldCircularArc("PrimarySingleSize_CenterCircle",
                   -2020, 0, 495, 0, 2.0 * M_PI, 10),
  FieldCircularArc("PrimarySingleSize_LeftFieldLeftPenaltyArc",
                   -1845, -2835, 795, 0.0, 0.5 * M_PI, 10),
  FieldCircularArc("PrimarySingleSize_LeftFieldRightPenaltyArc",
                   -2195, -2835, 795, 0.5 * M_PI, M_PI, 10),
  FieldCircularArc("PrimarySingleSize_RightFieldRightPenaltyArc",
                   -1845, 2835, 795, 1.5 * M_PI, 2.0 * M_PI, 10),
  FieldCircularArc("PrimarySingleSize_RightFieldLeftPenaltyArc",
                   -2195, 2835, 795, M_PI, 1.5 * M_PI, 10)
};

const GVector::vector2d<double> kCameraControlPoints[4][4] = {
  {
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(0.0, 4045.0),
    GVector::vector2d<double>(4045.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
  },

  {
    GVector::vector2d<double>(-4045.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(0.0, 3025.0),
    GVector::vector2d<double>(-4045.0, 3025.0),
  },

  {
    GVector::vector2d<double>(-4045.0, -3025.0),
    GVector::vector2d<double>(0.0, -3025.0),
    GVector::vector2d<double>(0.0, 0.0),
    GVector::vector2d<double>(-4045.0, 0.0),
  },

  {
    GVector::vector2d<double>(0.0, -3025.0),
    GVector::vector2d<double>(4045.0, -3025.0),
    GVector::vector2d<double>(4045.0, 0.0),
    GVector::vector2d<double>(0.0, 0.0),
  }
};

#endif

}  // namespace FieldConstantsRoboCup2012
