#ifndef ROBOTOUTLINE_H
#define ROBOTOUTLINE_H

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QVector>
#include <QGraphicsView>
#include <QPainterPath>

const int teamUnknown = 0;
const int teamBlue = 1;
const int teamYellow = 2;

class Robot : public QGraphicsPathItem
{
public:
    double orientation;     //In degrees
    int teamID;             //Team ID. 0 = blue, 1 = yellow
    int id;                 //ID of the robot in its team
    double x,y;
private:
    QBrush* brush;
    QPen* pen;
    QPainterPath robotOutline, robotID;
    QString robotLabel;

public:
    int key;
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    Robot();
    Robot(double _x, double _y, double _orientation, int _teamID, int _id, int _key);
    void SetPose(double _x, double _y, double _orientation);
};



class SoccerView : public QGraphicsView
{
    //Q_OBJECT

public:
    SoccerView();
    void AddRobot(Robot* robot);
    void UpdateRobot(double x, double y, double orientation, int team, int robotID, int key);


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

};


#endif // ROBOTOUTLINE_H
