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
  \file    ClientThreading.cpp
  \brief   C++ Implementation: ViewUpdateThread
  \author  Joydeep Biswas, Stefan Zickler (C) 2009
*/
//========================================================================
#include "ClientThreading.h"

ViewUpdateThread::ViewUpdateThread(SoccerView *_soccerView)
{
    soccerView = _soccerView;
    shutdownView = false;
}

int ViewUpdateThread::printRobotInfo(const SSL_DetectionRobot & robot, int teamID, int cameraID) {
    double x,y,orientation,conf =robot.confidence();
    int id, n=0;
    printf("CONF=%4.2f ", conf);
    if (robot.has_robot_id()) {
        id = robot.robot_id();
        printf("ID=%3d ",id);
    } else {
        printf("ID=N/A ");
        id = NA;
    }
    x = robot.x();
    y = robot.y();
    printf(" HEIGHT=%6.2f POS=<%9.2f,%9.2f> ",robot.height(),x,y);
    if (robot.has_orientation()) {
        orientation = robot.orientation();
        printf("ANGLE=%6.3f ",orientation);
    } else {
        orientation = NAOrientation;
        printf("ANGLE=N/A    ");
    }
    printf("RAW=<%8.2f,%8.2f>\n",robot.pixel_x(),robot.pixel_y());
    n = soccerView->UpdateRobot(x,y,orientation,teamID,id,cameraID,conf);
    return n;
}

void ViewUpdateThread::run()
{
    int n=0;
    client.open(false);
    while(!shutdownView){
        usleep(9000);
        if (client.receive(packet)) {
            printf("-----Received Wrapper Packet---------------------------------------------\n");
            //see if the packet contains a robot detection frame:
            if (packet.has_detection()) {
                SSL_DetectionFrame detection = packet.detection();
                //Display the contents of the robot detection results:

                int balls_n = detection.balls_size();
                int robots_blue_n =  detection.robots_blue_size();
                int robots_yellow_n =  detection.robots_yellow_size();

                //Ball info:
                QVector<QPointF> balls;

                for (int i = 0; i < balls_n; i++) {
                    QPointF p;
                    SSL_DetectionBall ball = detection.balls(i);
                    if (ball.confidence() > 0.0) {
                      p.setX(ball.x());
                      p.setY(ball.y());
                      balls.push_back(p);
                    }
                }
                if (balls.size() > 0) soccerView->UpdateBalls(balls);
                //Blue robot info:
                for (int i = 0; i < robots_blue_n; i++) {
                    SSL_DetectionRobot robot = detection.robots_blue(i);
                    n=printRobotInfo(robot, teamBlue, detection.camera_id());
                }

                //Yellow robot info:
                for (int i = 0; i < robots_yellow_n; i++) {
                    SSL_DetectionRobot robot = detection.robots_yellow(i);
                    n=printRobotInfo(robot, teamYellow, detection.camera_id());
                }

            }

            //see if packet contains geometry data:
            if (packet.has_geometry()) {
                const SSL_GeometryData & geom = packet.geometry();

                const SSL_GeometryFieldSize & field = geom.field();
                soccerView->LoadFieldGeometry((SSL_GeometryFieldSize&)field);

            }
        }

    }
}

void ViewUpdateThread::Terminate()
{
    shutdownView = true;
}
