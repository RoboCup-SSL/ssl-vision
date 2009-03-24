#include <QtGui>

#include "GraphicsPrimitives.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    SoccerView view;
    view.show();
    return app.exec();
}
