#include "GraphicsPrimitives.h"
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QWheelEvent>
#include "math.h"

Robot::Robot()
{
    Robot(0,0,0,teamUnknown,0,0,0);
}

Robot::Robot(double _x, double _y, double _orientation, int _teamID, int _id, int _key, double _conf)
{
    conf = _conf;
    key = _key;
    setVisible(true);
    setZValue(1);
    x = _x;
    y = _y;
    orientation = _orientation;
    teamID = _teamID;
    id = _id;

    robotOutline.moveTo(90,0);
    robotOutline.arcTo(-90,-90,180,180,0,270);
    robotOutline.closeSubpath();

    if(id!=idNA)
        robotLabel = QString(QByteArray((char *) &id, 4).toHex()).mid(1,1).toUpper();
    else
        robotLabel = "?";
    robotID.addText(-25,25,QFont("Courier",80,2,false),robotLabel);
    switch(teamID)
    {
        case teamBlue:
        {
            brush = new QBrush(QColor(0x41,0x7e,0xff,255),Qt::SolidPattern);
            pen = new QPen(QColor(0x12,0x3b,0xa0,255));
            break;
        }
        case teamYellow:
        {
            brush = new QBrush(QColor(0xff,0xf3,0x3e,255),Qt::SolidPattern);
            pen = new QPen(QColor(0xcc,0x9d,0x00,255));
            break;
        }
        default:
        {
            brush = new QBrush(QColor(150,150,150,255),Qt::SolidPattern);
            pen = new QPen(QColor(70,70,70,255));
            break;
        }
    }
    pen->setWidth(6);
    idPen = new QPen(Qt::black);
    idPen->setWidth(0);
    confPen = new QPen(Qt::white);
    confPen->setWidth(1);
    tStamp = GetTimeUSec();
}

Robot::~Robot()
{
    delete brush;
    delete pen;
    delete idPen;
}

QRectF Robot::boundingRect() const
{
    return QRectF(-95+x,-95-y,190,190);
}

void Robot::paint(QPainter *painter, const QStyleOptionGraphicsItem* , QWidget* )
{
    painter->translate(x,-y);
    painter->setPen(*pen);
    painter->setBrush(*brush);
    painter->rotate(-45-orientation);
    painter->drawPath(robotOutline);
    painter->rotate(45+orientation);
    painter->setPen(*idPen);
    painter->drawPath(robotID);
    painter->setPen(Qt::NoPen);
    painter->drawRect(-90,-130,((double)180)*conf,30);
    painter->setPen(*confPen);
    painter->setBrush(QBrush(Qt::white, Qt::NoBrush));
    painter->drawRect(-90,-130,180,30);
}

QPainterPath Robot::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 20, 20);
    return path;
}

void Robot::SetPose(double _x, double _y, double _orientation, double _conf)
{
    x = _x;
    y = _y;
    orientation = _orientation;
    conf = _conf;
    tStamp = GetTimeUSec();
}

SoccerView::SoccerView()
{
    LoadFieldGeometry();
    scene = new QGraphicsScene(this);
    setScene(scene);
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    scene->setBackgroundBrush(QBrush(QColor(0,0x91,0x19,255),Qt::SolidPattern));
    scene->setSceneRect(-3700,-2700,7400,5400);
    ConstructField();
    fieldBrush = new QBrush(Qt::NoBrush);
    fieldLinePen = new QPen();
    fieldLinePen->setColor(Qt::white);
    fieldLinePen->setWidth(10);
    fieldLinePen->setJoinStyle(Qt::MiterJoin);
    fieldItem = scene->addPath(field,*fieldLinePen,*fieldBrush);
    this->scale(0.14,0.14);
    this->setRenderHint(QPainter::Antialiasing, true);
    this->setDragMode(QGraphicsView::ScrollHandDrag);
    //QSizePolicy horPolicy(
    //this->setSizePolicy(horPolicy,vertPolicy);
    this->setGeometry(100,100,1036,756);
}

void SoccerView::ConstructField()
{
    field.moveTo(0,-field_width/2);
    field.lineTo(0,field_width/2);

    field.addEllipse(-center_circle_radius,-center_circle_radius,
                     2*center_circle_radius,2*center_circle_radius);

    field.moveTo(field_length/2,-field_width/2);
    field.lineTo(field_length/2,field_width/2);

    field.moveTo(-field_length/2,-field_width/2);
    field.lineTo(-field_length/2,field_width/2);

    field.moveTo(-field_length/2,-field_width/2);
    field.lineTo(field_length/2,-field_width/2);

    field.moveTo(-field_length/2,field_width/2);
    field.lineTo(field_length/2,field_width/2);

    field.moveTo(field_length/2,goal_width/2);
    field.lineTo((field_length/2+goal_depth),goal_width/2);
    field.lineTo((field_length/2+goal_depth),-goal_width/2);
    field.lineTo(field_length/2,-goal_width/2);
    field.moveTo((field_length/2-defense_radius),defense_stretch/2);
    field.lineTo((field_length/2-defense_radius),-defense_stretch/2);
    field.moveTo((field_length/2-defense_radius),defense_stretch/2);
    field.arcTo((field_length/2-defense_radius),-(defense_radius-defense_stretch/2),defense_radius*2,defense_radius*2,180,90);
    field.moveTo((field_length/2-defense_radius),-defense_stretch/2);
    field.arcTo((field_length/2-defense_radius),-(defense_radius+defense_stretch/2),defense_radius*2,defense_radius*2,180,-90);

    field.moveTo(-field_length/2,goal_width/2);
    field.lineTo(-(field_length/2+goal_depth),goal_width/2);
    field.lineTo(-(field_length/2+goal_depth),-goal_width/2);
    field.lineTo(-field_length/2,-goal_width/2);
    field.moveTo(-(field_length/2-defense_radius),defense_stretch/2);
    field.lineTo(-(field_length/2-defense_radius),-defense_stretch/2);
    field.moveTo(-(field_length/2-defense_radius),defense_stretch/2);
    field.arcTo(-(field_length/2+defense_radius),-(defense_radius-defense_stretch/2),defense_radius*2,defense_radius*2,0,-90);
    field.moveTo(-(field_length/2-defense_radius),-defense_stretch/2);
    field.arcTo(-(field_length/2+defense_radius),-(defense_radius+defense_stretch/2),defense_radius*2,defense_radius*2,0,90);
}

void SoccerView::scaleView(qreal scaleFactor)
{
    qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}

void SoccerView::wheelEvent(QWheelEvent *event)
{
    scaleView(pow((double)2, event->delta() / 540.0));
}

void SoccerView::AddRobot(Robot *robot)
{
    robots.append(robot);
    scene->addItem(robot);
}

int SoccerView::UpdateRobot(double x, double y, double orientation, int team, int robotID, int key, double conf)
{
    orientation *= 180.0/M_PI;
    bool found = false;
    Robot *currentRobot;
    for(int i=0; i<robots.size() && robotID!=idNA ; i++)
    {
        currentRobot = robots[i];
        if(currentRobot->id==robotID && currentRobot->teamID==team && currentRobot->key==key)
        {
            currentRobot->SetPose(x,y,orientation,conf);
            currentRobot->update(currentRobot->boundingRect());
            found = true;
            //this->update();
            break;
        }
    }
    if(!found)
    {
        AddRobot(new Robot(x,y,orientation,team,robotID,key,conf));
    }
    return robots.size();
}

void SoccerView::LoadFieldGeometry()
{
    this->line_width = FieldConstantsRoboCup2009::line_width;
    this->field_length = FieldConstantsRoboCup2009::field_length;
    this->field_width = FieldConstantsRoboCup2009::field_width;
    this->boundary_width = FieldConstantsRoboCup2009::boundary_width;
    this->referee_width = FieldConstantsRoboCup2009::referee_width;
    this->goal_width = FieldConstantsRoboCup2009::goal_width;
    this->goal_depth = FieldConstantsRoboCup2009::goal_depth;
    this->goal_wall_width = FieldConstantsRoboCup2009::goal_wall_width;
    this->center_circle_radius = FieldConstantsRoboCup2009::center_circle_radius;
    this->defense_radius = FieldConstantsRoboCup2009::defense_radius;
    this->defense_stretch = FieldConstantsRoboCup2009::defense_stretch;
    this->free_kick_from_defense_dist = FieldConstantsRoboCup2009::free_kick_from_defense_dist;
    this->penalty_spot_from_field_line_dist = FieldConstantsRoboCup2009::penalty_spot_from_field_line_dist;
    this->penalty_line_from_spot_dist = FieldConstantsRoboCup2009::penalty_line_from_spot_dist;
}

void SoccerView::LoadFieldGeometry(SSL_GeometryFieldSize &fieldSize)
{
    this->line_width = fieldSize.line_width();
    this->field_length = fieldSize.field_length();
    this->field_width = fieldSize.field_width();
    this->boundary_width = fieldSize.boundary_width();
    this->referee_width = fieldSize.referee_width();
    this->goal_width = fieldSize.goal_width();
    this->goal_depth = fieldSize.goal_depth();
    this->goal_wall_width = fieldSize.goal_wall_width();
    this->center_circle_radius = fieldSize.center_circle_radius();
    this->defense_radius = fieldSize.defense_radius();
    this->defense_stretch = fieldSize.defense_stretch();
    this->free_kick_from_defense_dist = fieldSize.free_kick_from_defense_dist();
    this->penalty_spot_from_field_line_dist = fieldSize.penalty_spot_from_field_line_dist();
    this->penalty_line_from_spot_dist = fieldSize.penalty_line_from_spot_dist();
}

void SoccerView::PruneRobots()
{
    unsigned long int tnow = GetTimeUSec();
    for(int i=0;i<robots.size();i++)
    {
        Robot* robot = robots[i];
        if(tnow-robot->tStamp>1e6 || (tnow-robot->tStamp>1e5 && robot->id==idNA) )
        {
            robots.remove(i);
            scene->removeItem(robot);
            //delete robot;
        }
    }
}
