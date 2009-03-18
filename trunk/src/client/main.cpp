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
#include "robocup_ssl_client.h"

#include "messages_robocup_ssl_detection.pb.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "messages_robocup_ssl_wrapper.pb.h"

int main(int argc, char *argv[])
{
  /*QApplication app(argc, argv);

  MainWindow mainWin;
 
  //if desired, launch a particular style:
  // app.setStyle(new QPlastiqueStyle());
  // app.setStyle(new QCleanlooksStyle());
  mainWin.show();
  mainWin.init();
  int retval = app.exec();*/


  RoboCupSSLClient client;
  client.open();
  SSL_WrapperPacket packet;

  while(true) {
    if (client.receive(packet)) {
      printf("RECEIVED SSL WRAPPER PACKET\n");
    }
  }

  return 0;
}
