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
  \file    sslField.cpp
  \brief   Class for drawing the current field, robots, and the ball; reusing parts of graphicalClient
  \author  Tim Laue (C) 2010
*/
//========================================================================

#include "sslField.h"
#include "field_default_constants.h"
#include <QPainter>
#include <cmath>
#include <iostream>


SSLField::SSLField(GClientBlackBoard& blackBoard):blackBoard(blackBoard)
{
    loadFieldGeometryDefaults();
    robotOutline.moveTo ( 90,0 );
    robotOutline.arcTo ( -90,-90,180,180,0,270 );
    robotOutline.closeSubpath();
    robotOutlineCircle.addEllipse ( -90,-90,180,180 );
}


QRectF SSLField::boundingRect() const
{
    return QRectF(0,0,0,0);
}


void SSLField::advance(int step)
{
    if (!step)
        return;
    if(blackBoard.isGeometryNew())
    {
        blackBoard.getGeometry(geometry);
        loadFieldGeometryFromNetwork(geometry.field());
    }
    if(blackBoard.isDetectionNew())
    {
        blackBoard.getDetection(detection);
    }  
}


void SSLField::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // The field:
    paintField(painter);
    // The robots
    for (int i = 0; i < detection.robots_blue_size(); i++) 
        paintRobot(painter, detection.robots_blue(i), QColor(0,0,255));
    for (int i = 0; i < detection.robots_yellow_size(); i++) 
        paintRobot(painter, detection.robots_yellow(i), QColor(255,255,0));
    // The ball(s)
    for (int i = 0; i < detection.balls_size(); i++) 
    {
        SSL_DetectionBall ball = detection.balls(i);
        painter->setBrush(QColor(255,128,128));
        painter->setPen(QColor(255,128,128));
        painter->drawEllipse(QPoint(ball.x(),-ball.y()),21,21);
    }
    // Fit field in view
    const QSize size(parentView->size());
    int viewWidth = (field_length+2*goal_depth+2*referee_width)*1.2;
    int viewHeight =(field_width+2*referee_width)*1.2;
    float xScale = float(size.width()) / viewWidth;
    float yScale = float(size.height()) / viewHeight;
    float scale = xScale < yScale ? xScale : yScale;
    parentView->setTransform(QTransform(scale, 0, 0, scale, size.width() / 2., size.height() / 2.));
}


void SSLField::paintRobot(QPainter *painter, const SSL_DetectionRobot& robot, const QColor& color)
{
     // Save transformation:
     QTransform oldTransformation = painter->worldTransform();
     // Draw robot shape:
     painter->translate(robot.x(),-robot.y());
     painter->setBrush(QColor(0,0,0));    
     painter->setPen(QColor(0,0,0)); 
     double robotRotation = 1000;
     if (robot.has_orientation())
     {
         robotRotation = robot.orientation() *180.0/M_PI;
     }
     if (fabs(robotRotation) <360)
     {
         painter->rotate(-45-robotRotation);
         painter->drawPath(robotOutline);
         painter->rotate(45+robotRotation);
     }
     else
     {
         painter->drawPath(robotOutlineCircle);
     }
     // Draw team marker
     painter->setBrush(color);    
     painter->setPen(color); 
     painter->drawEllipse(QPoint(0,0),25,25);
     // Draw confidence
     painter->setBrush(color);    
     painter->setPen(color); 
     painter->drawRect ( -90,-190, ( int ) ( ( ( double ) 180 ) * robot.confidence() ),80 );
     // Draw id
     QString robotId("?");
     if(robot.has_robot_id())
        robotId.setNum(robot.robot_id(),16);
     painter->setBrush(QColor(0,0,0));    
     painter->setPen(QColor(0,0,0)); 
     painter->setFont(QFont ( "Courier",80,2,false ));
     painter->drawText(-90,-210,1000,1000,0,robotId);
     // Reset transformation
     painter->setTransform(oldTransformation);
/*
  painter->setPen ( Qt::NoPen );
  painter->setBrush ( *brush );
  painter->drawRect ( -90,-130, ( int ) ( ( ( double ) 180 ) *conf ),30 );
  painter->setPen ( *pen );
  painter->setBrush ( QBrush ( Qt::white, Qt::NoBrush ) );
  painter->drawRect ( -90,-130,180,30 );
  */
}


void SSLField::paintField(QPainter* painter)
{
    QPainterPath fieldPath;
    fieldPath.moveTo ( 0,-field_width/2 );
    fieldPath.lineTo ( 0,field_width/2 );
    fieldPath.addEllipse ( -center_circle_radius,-center_circle_radius,
                      2*center_circle_radius,2*center_circle_radius );
    fieldPath.moveTo ( field_length/2,-field_width/2 );
    fieldPath.lineTo ( field_length/2,field_width/2 );
    fieldPath.moveTo ( -field_length/2,-field_width/2 );
    fieldPath.lineTo ( -field_length/2,field_width/2 );
    fieldPath.moveTo ( -field_length/2,-field_width/2 );
    fieldPath.lineTo ( field_length/2,-field_width/2 );
    fieldPath.moveTo ( -field_length/2,field_width/2 );
    fieldPath.lineTo ( field_length/2,field_width/2 );
    fieldPath.moveTo ( field_length/2,goal_width/2 );
    fieldPath.lineTo ( ( field_length/2+goal_depth ),goal_width/2 );
    fieldPath.lineTo ( ( field_length/2+goal_depth ),-goal_width/2 );
    fieldPath.lineTo ( field_length/2,-goal_width/2 );
    fieldPath.moveTo ( ( field_length/2-defense_radius ),defense_stretch/2 );
    fieldPath.lineTo ( ( field_length/2-defense_radius ),-defense_stretch/2 );
    fieldPath.moveTo ( ( field_length/2-defense_radius ),defense_stretch/2 );
    fieldPath.arcTo ( ( field_length/2-defense_radius ),- ( defense_radius-defense_stretch/2 ),defense_radius*2,defense_radius*2,180,90 );
    fieldPath.moveTo ( ( field_length/2-defense_radius ),-defense_stretch/2 );
    fieldPath.arcTo ( ( field_length/2-defense_radius ),- ( defense_radius+defense_stretch/2 ),defense_radius*2,defense_radius*2,180,-90 );
    fieldPath.moveTo ( -field_length/2,goal_width/2 );
    fieldPath.lineTo ( - ( field_length/2+goal_depth ),goal_width/2 );
    fieldPath.lineTo ( - ( field_length/2+goal_depth ),-goal_width/2 );
    fieldPath.lineTo ( -field_length/2,-goal_width/2 );
    fieldPath.moveTo ( - ( field_length/2-defense_radius ),defense_stretch/2 );
    fieldPath.lineTo ( - ( field_length/2-defense_radius ),-defense_stretch/2 );
    fieldPath.moveTo ( - ( field_length/2-defense_radius ),defense_stretch/2 );
    fieldPath.arcTo ( - ( field_length/2+defense_radius ),- ( defense_radius-defense_stretch/2 ),defense_radius*2,defense_radius*2,0,-90 );
    fieldPath.moveTo ( - ( field_length/2-defense_radius ),-defense_stretch/2 );
    fieldPath.arcTo ( - ( field_length/2+defense_radius ),- ( defense_radius+defense_stretch/2 ),defense_radius*2,defense_radius*2,0,90 );
    QPen linePen(QColor(255,255,255));
    linePen.setWidth(line_width);
    painter->setBrush(QColor(255,255,255));
    painter->setPen(linePen);
    QPainterPathStroker stroker;
    QPainterPath path2 = stroker.createStroke(fieldPath);
    painter->drawPath(path2);
}


void SSLField::loadFieldGeometryDefaults()
{
    line_width = FieldConstantsRoboCup2009::line_width;
    field_length = FieldConstantsRoboCup2009::field_length;
    field_width = FieldConstantsRoboCup2009::field_width;
    boundary_width = FieldConstantsRoboCup2009::boundary_width;
    referee_width = FieldConstantsRoboCup2009::referee_width;
    goal_width = FieldConstantsRoboCup2009::goal_width;
    goal_depth = FieldConstantsRoboCup2009::goal_depth;
    goal_wall_width = FieldConstantsRoboCup2009::goal_wall_width;
    center_circle_radius = FieldConstantsRoboCup2009::center_circle_radius;
    defense_radius = FieldConstantsRoboCup2009::defense_radius;
    defense_stretch = FieldConstantsRoboCup2009::defense_stretch;
    free_kick_from_defense_dist = FieldConstantsRoboCup2009::free_kick_from_defense_dist;
    penalty_spot_from_field_line_dist = FieldConstantsRoboCup2009::penalty_spot_from_field_line_dist;
    penalty_line_from_spot_dist = FieldConstantsRoboCup2009::penalty_line_from_spot_dist;
}


void SSLField::loadFieldGeometryFromNetwork(const SSL_GeometryFieldSize &fieldSize)
{
    line_width = fieldSize.line_width();
    field_length = fieldSize.field_length();
    field_width = fieldSize.field_width();
    boundary_width = fieldSize.boundary_width();
    referee_width = fieldSize.referee_width();
    goal_width = fieldSize.goal_width();
    goal_depth = fieldSize.goal_depth();
    goal_wall_width = fieldSize.goal_wall_width();
    center_circle_radius = fieldSize.center_circle_radius();
    defense_radius = fieldSize.defense_radius();
    defense_stretch = fieldSize.defense_stretch();
    free_kick_from_defense_dist = fieldSize.free_kick_from_defense_dist();
    penalty_spot_from_field_line_dist = fieldSize.penalty_spot_from_field_line_dist();
    penalty_line_from_spot_dist = fieldSize.penalty_line_from_spot_dist();
}

