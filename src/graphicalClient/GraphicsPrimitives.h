#ifndef ROBOTOUTLINE_H
#define ROBOTOUTLINE_H

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QVector>
#include <QGraphicsView>\
#include <QPainterPath>

const int teamUnknown = 0;
const int teamBlue = 1;
const int teamYellow = 2;

class Robot : public QGraphicsPathItem
{
private:
    double orientation;     //In degrees
    int teamID;             //Team ID. 0 = blue, 1 = yellow
    int id;                 //ID of the robot in its team
    double x,y;
    QBrush* brush;
    QPen* pen;
    QPainterPath robotOutline, robotID;
    QString robotLabel;

public:
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    Robot();
    Robot(double _x, double _y, double _orientation, int _teamID, int _id);
    void SetPose(double _x, double _y, double _orientation);
};



class SoccerView : public QGraphicsView
{
    //Q_OBJECT

public:
    SoccerView();


protected:
    //void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    //void drawBackground(QPainter *painter, const QRectF &rect);
    void scaleView(qreal scaleFactor);

private:
    //Robots
    QVector<Robot> robots;
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

};

#endif // ROBOTOUTLINE_H
