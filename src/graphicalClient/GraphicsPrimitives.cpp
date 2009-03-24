#include "GraphicsPrimitives.h"
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QWheelEvent>
#include "math.h"

Robot::Robot()
{
    Robot(0,0,0,teamUnknown,0,0);
}
Robot::Robot(double _x, double _y, double _orientation, int _teamID, int _id, int _key)
{
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

    robotLabel = QString(QByteArray((char *) &id, 4).toHex()).mid(1,1).toUpper();
    robotID.addText(-25,25,QFont("Courier New",80,2,false),robotLabel);
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

}

QRectF Robot::boundingRect() const
{
    return QRectF(-80+x,-80-y,160,160);
}

void Robot::paint(QPainter *painter, const QStyleOptionGraphicsItem* , QWidget* )
{
    painter->translate(x,-y);
    painter->setPen(*pen);
    painter->setBrush(*brush);
    painter->rotate(-45-orientation);
    painter->drawPath(robotOutline);
    painter->rotate(45+orientation);
    painter->setPen(QPen(Qt::black));
    painter->drawPath(robotID);
}

QPainterPath Robot::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 20, 20);
    return path;
}

void Robot::SetPose(double _x, double _y, double _orientation)
{
    x = _x;
    y = _y;
    orientation = _orientation;
}

SoccerView::SoccerView()
{
    scene = new QGraphicsScene(this);
    setScene(scene);

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
    field.moveTo(0,-2025);
    field.lineTo(0,2025);

    field.addEllipse(-500,-500,1000,1000);

    field.moveTo(3025,-2025);
    field.lineTo(3025,2025);

    field.moveTo(-3025,-2025);
    field.lineTo(-3025,2025);

    field.moveTo(-3025,-2025);
    field.lineTo(3025,-2025);

    field.moveTo(-3025,2025);
    field.lineTo(3025,2025);

    field.moveTo(3025,350);
    field.lineTo(3205,350);
    field.lineTo(3205,-350);
    field.lineTo(3025,-350);
    field.moveTo(2525,175);
    field.lineTo(2525,-175);
    field.moveTo(2525,175);
    field.arcTo(2525,-325,1000,1000,180,90);
    field.moveTo(2525,-175);
    field.arcTo(2525,-675,1000,1000,180,-90);

    field.moveTo(-3025,350);
    field.lineTo(-3205,350);
    field.lineTo(-3205,-350);
    field.lineTo(-3025,-350);
    field.moveTo(-2525,175);
    field.lineTo(-2525,-175);
    field.moveTo(-2525,175);
    field.arcTo(-3525,-325,1000,1000,0,-90);
    field.moveTo(-2525,-175);
    field.arcTo(-3525,-675,1000,1000,0,90);
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

void SoccerView::UpdateRobot(double x, double y, double orientation, int team, int robotID, int key)
{
    bool found = false;
    Robot *currentRobot;
    for(int i=0; i<robots.size(); i++)
    {
        currentRobot = robots[i];
        if(currentRobot->id==robotID && currentRobot->teamID==team && currentRobot->key==key)
        {
            currentRobot->SetPose(x,y,orientation);
            found = true;
        }
    }
    if(!found)
    {
        AddRobot(new Robot(x,y,orientation,team,robotID,key));
    }
    return;
}

