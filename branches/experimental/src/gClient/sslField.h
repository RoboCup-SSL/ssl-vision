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
  \file    sslField.h
  \brief   Class for drawing the current field, robots, and the ball
  \author  Tim Laue (C) 2010
*/
//========================================================================

#ifndef SSL_FIELD_H
#define SSL_FIELD_H

#include "gClientBlackBoard.h"
#include <QtGui>


class SSLField : public QGraphicsItem
{
public:
    SSLField(GClientBlackBoard& blackBoard);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    void setParentView(QGraphicsView* parentView)
    {
        this->parentView = parentView;
    }

protected:
    void advance(int step);
    void paintRobot(QPainter *painter, const SSL_DetectionRobot& robot, const QColor& teamColor);
    void paintField(QPainter *painter);

private:
    void loadFieldGeometryDefaults();
    void loadFieldGeometryFromNetwork(const SSL_GeometryFieldSize &fieldSize);
    
    GClientBlackBoard& blackBoard;
    SSL_DetectionFrame detection;
    SSL_GeometryData geometry;
    QPainterPath robotOutline, robotOutlineCircle;
    QGraphicsView* parentView;
    
    //Field dimensions and geometry
    double line_width;
    double field_length;
    double field_width;
    double boundary_width;
    double referee_width;
    double goal_width;
    double goal_depth;
    double goal_wall_width;
    double center_circle_radius;
    double defense_radius;
    double defense_stretch;
    double free_kick_from_defense_dist;
    double penalty_spot_from_field_line_dist;
    double penalty_line_from_spot_dist;
};

#endif

