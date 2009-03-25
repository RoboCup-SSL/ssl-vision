#ifndef GRAPHICS_PRIMITIVES_H
#define GRAPHICS_PRIMITIVES_H

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QVector>
#include <QGraphicsView>
#include <QPainterPath>
#include <QApplication>
#include <QThread>
#include "field_default_constants.h"
#include "robocup_ssl_client.h"
#include "timer.h"

const int teamUnknown = 0;
const int teamBlue = 1;
const int teamYellow = 2;
const int idNA = 0xffff;

class Robot : public QGraphicsPathItem
{
public:
    double orientation;     //In degrees
    int teamID;             //Team ID. 0 = blue, 1 = yellow
    int id;                 //ID of the robot in its team
    double x,y;
    double conf;

private:
    QBrush *brush;
    QPen *pen, *idPen, *confPen;
    QPainterPath robotOutline, robotID;
    QString robotLabel;

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
    void AddRobot(Robot* robot);
    int UpdateRobot(double x, double y, double orientation, int team, int robotID, int key, double conf);
    void LoadFieldGeometry();
    void LoadFieldGeometry(SSL_GeometryFieldSize &fieldSize);
    void PruneRobots();

protected:
    //void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    //void drawBackground(QPainter *painter, const QRectF &rect);
    void scaleView(qreal scaleFactor);

private:
    //Robots
    QVector<Robot*> robots;
    //balls
    QVector<QPointF> balls;
    //scene
    QGraphicsScene* scene;
    //field
    QPainterPath field;
    QGraphicsPathItem* fieldItem;
    void ConstructField();
    //brushes and pens
    QBrush *fieldBrush, *ballBrush;
    QPen *fieldPen, *fieldLinePen, *ballPen;

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
