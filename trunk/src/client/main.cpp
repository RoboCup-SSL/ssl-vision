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
      printf("-----Received Wrapper Packet---------------------------------------------\n");
      //see if the packet contains a robot detection frame:
      if (packet.has_detection()) {
        SSL_DetectionFrame detection = packet.detection();
        //Display the contents of the robot detection results:

        //Frame info:
        printf("Detection Frame: Camera ID=%d FRAME=%d TIMESTAMP=%.4f\n",detection.camera_id(),detection.frame_number(),detection.timestamp());
        int balls_n = detection.balls_size();
        int robots_blue_n =  detection.robots_blue_size();
        int robots_yellow_n =  detection.robots_yellow_size();

        //Ball info:
        for (int i = 0; i < balls_n; i++) {
          SSL_DetectionBall ball = detection.balls(i);
          printf("-Ball (%2d/%2d): CONF=%4.2f POS=<%9.2f,%9.2f> Z=%7.2f RAW=<%8.2f,%8.2f>\n", i+1, balls_n, ball.confidence(),ball.x(),ball.y(),ball.has_z() ? ball.z() : 0.0,ball.pixel_x(),ball.pixel_y());
        }

        //Blue robot info:
        for (int i = 0; i < robots_blue_n; i++) {
          SSL_DetectionRobot robot = detection.robots_blue(i);
          printf("-Robot(B) (%2d/%2d): CONF=%4.2f ID=%2d HEIGHT=%6.2f POS=<%9.2f,%9.2f> ANGLE=%6.3f RAW=<%8.2f,%8.2f>\n", i+1, robots_blue_n, robot.confidence(),robot.robot_id(),robot.height(),robot.x(),robot.y(),robot.orientation(),robot.pixel_x(),robot.pixel_y());
        }

        //Yellow robot info:
        for (int i = 0; i < robots_yellow_n; i++) {
          SSL_DetectionRobot robot = detection.robots_yellow(i);
          printf("-Robot(Y) (%2d/%2d): CONF=%4.2f ID=%2d HEIGHT=%6.2f POS=<%9.2f,%9.2f> ANGLE=%6.3f RAW=<%8.2f,%8.2f>\n", i+1, robots_yellow_n, robot.confidence(),robot.robot_id(),robot.height(),robot.x(),robot.y(),robot.orientation(),robot.pixel_x(),robot.pixel_y());
        }

      }
    }
  }

  return 0;
}
