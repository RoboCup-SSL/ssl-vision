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
  \author  Stefan Zickler / Tim Laue, (C) 2009
*/
//========================================================================

#include <vector>
#include "field.h"
#include "geometry.h"

#ifndef FIELD_DEFAULT_CONSTANTS_H
#define FIELD_DEFAULT_CONSTANTS_H

class FieldLine;
class FieldCircularArc;

namespace FieldConstantsRoboCup2018A {
    const double kFieldLength = 12000.0;
    const double kFieldWidth = 9000.0;
    const double kGoalWidth = 1800.0;
    const double kGoalDepth = 180.0;
    const double kGoalHeight = 155.0;
    const double kBoundaryWidth = 300.0;
    const double kLineThickness = 10.0;
    const double kPenaltyAreaDepth = 1800.0;
    const double kPenaltyAreaWidth = 3600.0;
    const double kCenterCircleRadius = 500.0;
    const double kGoalLineToPenaltyMark = 8000.0;
    const double kBallRadius = 21.5;
    const double kMaxRobotRadius = 90.0;
    const int kNumCamerasTotal = 2;
    const int kNumCamerasLocal = 1;
}

namespace FieldConstantsRoboCup2018B {
    const double kFieldLength = 9000.0;
    const double kFieldWidth = 6000.0;
    const double kGoalWidth = 1000.0;
    const double kGoalDepth = 180.0;
    const double kGoalHeight = 155.0;
    const double kBoundaryWidth = 300.0;
    const double kLineThickness = 10.0;
    const double kPenaltyAreaDepth = 1000.0;
    const double kPenaltyAreaWidth = 2000.0;
    const double kCenterCircleRadius = 500.0;
    const double kGoalLineToPenaltyMark = 6000.0;
    const double kBallRadius = 21.5;
    const double kMaxRobotRadius = 90.0;
    const int kNumCamerasTotal = 1;
    const int kNumCamerasLocal = 1;
}

#endif // FIELD_DEFAULT_CONSTANTS_H
