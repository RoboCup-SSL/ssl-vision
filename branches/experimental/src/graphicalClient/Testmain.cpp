#include <QtGui>
#include <QApplication>
#include "GraphicsPrimitives.h"

SoccerView *view;

void Init()
{
    Robot *r1, *r2, *r3;
    /*
    r1 = new Robot(1000,200,60,teamYellow,0,0),
    r2 = new Robot(-100,260,23,teamBlue,10,0),
    r3 = new Robot(-2000,-1200,287,teamUnknown,11,0);
    view->AddRobot(r1);
    view->AddRobot(r2);
    view->AddRobot(r3);
    view->AddRobot(new Robot(762,-1233,0,teamBlue,8));
    */
    view->UpdateRobot(1000,200,60,teamYellow,0,0),
    view->UpdateRobot(-100,260,23,teamBlue,10,0),
    view->UpdateRobot(-2000,-1200,287,teamUnknown,11,0);


    view->UpdateRobot(-2000,-1200,200,teamUnknown,11,0);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    view = new SoccerView();
    Init();
    view->show();

    return app.exec();
}

