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
\brief   Main Entry point for the graphicalClient binary`
\author  Joydeep Biswas (C) 2011
*/
//========================================================================

#include <stdio.h>
#include <QtGui>
#include <QApplication>
#include "soccerview.h"
#include "timer.h"

GLSoccerView *view;

bool runApp = true;

class MyThread : public QThread
{  
protected:
  void run()
  {
    static const double minDuration = 0.01; //100FPS
    RoboCupSSLClient client(m_port);
    client.open(false);
    SSL_WrapperPacket packet;
    while(runApp) {
      while (client.receive(packet)) {
        if (packet.has_detection()) {
          SSL_DetectionFrame detection = packet.detection();
          view->updateDetection(detection);
        }
        if (packet.has_geometry()) {
          view->updateFieldGeometry(packet.geometry().field());
        }
      }
      Sleep(minDuration);
    }
  }
  
public:
  MyThread(int port, QObject* parent = 0): QThread(parent), m_port(port) {}
  ~MyThread(){}

private:
  int m_port;
};

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QStringList arguments = QCoreApplication::arguments();

  const int portNumber = arguments.size() > 1 ? arguments[1].toInt() : 10006;
  view = new GLSoccerView();
  view->show();
  MyThread thread(portNumber);
  thread.start();
  int retVal = app.exec();
  runApp = false;
  thread.wait();
  return retVal;
}

