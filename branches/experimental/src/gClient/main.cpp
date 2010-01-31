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
  \file    main.cpp
  \brief   The ssl-vision gClient application entry point. Partially based on text client.
  \author  Tim Laue (C) 2010
*/
//========================================================================

#include "sslField.h"
#include "sslReceiverThread.h"
#include "gClientBlackBoard.h"


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QGraphicsScene scene;
    scene.setSceneRect(-3700, -2700, 7400, 5400);
    scene.setItemIndexMethod(QGraphicsScene::NoIndex);

    GClientBlackBoard gClientBlackBoard;
    SSLField sslField(gClientBlackBoard);
    scene.addItem(&sslField);

    SSLReceiverThread sslReceiverThread(gClientBlackBoard);
    sslReceiverThread.start();

    QGraphicsView view(&scene);
    sslField.setParentView(&view);
    view.setRenderHint(QPainter::Antialiasing);
    view.setBackgroundBrush(QColor(27,158,56));
    view.setCacheMode(QGraphicsView::CacheBackground);
    view.setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    view.setDragMode(QGraphicsView::ScrollHandDrag);

    view.setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "SSL-Vision gClient"));
    view.resize(740, 540);
    view.fitInView(-3700, -2700, 7400, 5400);
    view.show();

    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &scene, SLOT(advance()));
    QObject::connect(&timer, SIGNAL(timeout()), &view, SLOT(invalidateScene()));
    timer.start(1000 / 33); // update at 30Hz

    // Try to properly exit the application. Not possible in case of no connection
    int returnValue = app.exec();
    sslReceiverThread.setStopReceiving();
    if(!sslReceiverThread.wait(200))
        sslReceiverThread.terminate();
    return returnValue;
}

