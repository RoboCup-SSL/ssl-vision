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
  \brief   The ssl-vision application entry point.
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

//#include <QApplication>
//#include <QCleanlooksStyle>
//#include <QPlastiqueStyle>
//#include "mainwindow.h"

#include <stdio.h>
#include <QThread>
#include <QtGui>
#include "GraphicsPrimitives.h"
#include "ClientThreading.h"

#include "messages_robocup_ssl_detection.pb.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "messages_robocup_ssl_wrapper.pb.h"

SoccerView *view;
QApplication *app;


int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    

    app = new QApplication(argc,argv);
    view = new SoccerView();
    printf("strobe\n");
    view->show();

    ViewUpdateThread thread(view);
    thread.start(QThread::NormalPriority);
    app->exec();
    thread.Terminate();


    return 0;
}
