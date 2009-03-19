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

void printRobotInfo(const SSL_DetectionRobot & robot) {
  printf("CONF=%4.2f ", robot.confidence());
  if (robot.has_robot_id()) {
    printf("ID=%3d ",robot.robot_id());
  } else {
    printf("ID=N/A ");
  }
  printf(" HEIGHT=%6.2f POS=<%9.2f,%9.2f> ",robot.height(),robot.x(),robot.y());
  if (robot.has_orientation()) {
    printf("ANGLE=%6.3f ",robot.orientation());
  } else {
    printf("ANGLE=N/A    ");
  }
  printf("RAW=<%8.2f,%8.2f>\n",robot.pixel_x(),robot.pixel_y());
}


int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

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
          printf("-Ball (%2d/%2d): CONF=%4.2f POS=<%9.2f,%9.2f> ", i+1, balls_n, ball.confidence(),ball.x(),ball.y());
          if (ball.has_z()) {
            printf("Z=%7.2f ",ball.z());
          } else {
            printf("Z=N/A   ");
          }
          printf("RAW=<%8.2f,%8.2f>\n",ball.pixel_x(),ball.pixel_y());
        }

        //Blue robot info:
        for (int i = 0; i < robots_blue_n; i++) {
          SSL_DetectionRobot robot = detection.robots_blue(i);
          printf("-Robot(B) (%2d/%2d): ",i+1, robots_blue_n);
          printRobotInfo(robot);
        }

        //Yellow robot info:
        for (int i = 0; i < robots_yellow_n; i++) {
          SSL_DetectionRobot robot = detection.robots_yellow(i);
          printf("-Robot(Y) (%2d/%2d): ",i+1, robots_yellow_n);
          printRobotInfo(robot);
        }

      }
    }
  }

  return 0;
}
