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
  \file    GraphicsPrimitives.cpp
  \brief   C++ Interface: Robot, SoccerView
  \author  Joydeep Biswas (C) 2009
*/
//========================================================================

#ifndef GRAPHICS_PRIMITIVES_H
#define GRAPHICS_PRIMITIVES_H

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QVector>
#include <QGraphicsView>
#include <QPainterPath>
#include <QApplication>
#include <QThread>
#include <QGLWidget>
#include <QMutex>
#include "field_default_constants.h"
#include "robocup_ssl_client.h"
#include "timer.h"

const int teamUnknown = 0;
const int teamBlue = 1;
const int teamYellow = 2;
const int NA = 0xffff;
const double NAOrientation = 1000.0;

class Robot : public QGraphicsPathItem
{
public:
    double orientation;     //In degrees
    int teamID;             //Team ID. 0 = blue, 1 = yellow
    int id;                 //ID of the robot in its team
    double x,y;
    double conf;
    QString robotLabel;

private:
    QBrush *brush;
    QPen *pen, *idPen, *confPen;
    QPainterPath robotOutline, robotOutlineCircle, robotID;

public:
    int key;
    QRectF boundingRect() const;
    QPainterPath shape() const;
    unsigned long int tStamp;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    Robot();
    Robot(double _x, double _y, double _orientation, int _teamID, int _id, int _key, double _conf);
    void SetPose(double _x, double _y, double _orientation, double _conf);
    ~Robot();
};



class SoccerView : public QGraphicsView
{
    //Q_OBJECT

public:
    SoccerView();
    ~SoccerView();
    void AddRobot(Robot* robot);
    int UpdateRobot(double x, double y, double orientation, int team, int robotID, int key, double conf);
    int UpdateBalls(QVector<QPointF> &_balls);
    void LoadFieldGeometry();
    void LoadFieldGeometry(SSL_GeometryFieldSize &fieldSize);
    void PruneRobots();

protected:
    //void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    //void drawBackground(QPainter *painter, const QRectF &rect);
    void scaleView(qreal scaleFactor);
    void timerEvent(QTimerEvent*);
    QMutex drawMutex;

private:
    //Robots
    QVector<Robot*> robots;
    //balls
    QVector<QPointF> balls;
    QVector<QGraphicsEllipseItem*> ballItems;
    //scene
    QGraphicsScene* scene;
    //field
    QPainterPath *field;
    QGraphicsPathItem *fieldItem;
    void ConstructField();
    //brushes and pens
    QBrush *fieldBrush, *ballBrush;
    QPen *fieldPen, *fieldLinePen, *ballPen;
    QGLWidget* glWidget;
    bool shutdownSoccerView;

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

#endif //GRAPHICS_PRIMITIVES_H
