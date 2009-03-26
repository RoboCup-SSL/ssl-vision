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
                double t_now = GetTimeSec();

                printf("-[Detection Data]-------\n");
                //Frame info:
                printf("Camera ID=%d FRAME=%d TIMESTAMP=%.4f\n",detection.camera_id(),detection.frame_number(),detection.timestamp());

                printf("Total Vision Latency (assuming synched system clock) %7.3fms\n",(t_now-detection.timestamp())*1000.0);
                int balls_n = detection.balls_size();
                int robots_blue_n =  detection.robots_blue_size();
                int robots_yellow_n =  detection.robots_yellow_size();

                //Ball info:
                QVector<QPointF> balls(balls_n);
                for (int i = 0; i < balls_n; i++) {
                    SSL_DetectionBall ball = detection.balls(i);
                    balls[i].setX(ball.x());
                    balls[i].setY(ball.y());
                    printf("-Ball (%2d/%2d): CONF=%4.2f POS=<%9.2f,%9.2f> ", i+1, balls_n, ball.confidence(),ball.x(),ball.y());
                    if (ball.has_z()) {
                        printf("Z=%7.2f ",ball.z());
                    } else {
                        printf("Z=N/A   ");
                    }
                    printf("RAW=<%8.2f,%8.2f>\n",ball.pixel_x(),ball.pixel_y());
                }
                soccerView->UpdateBalls(balls);
                //Blue robot info:
                for (int i = 0; i < robots_blue_n; i++) {
                    SSL_DetectionRobot robot = detection.robots_blue(i);
                    printf("-Robot(B) (%2d/%2d): ",i+1, robots_blue_n);
                    n=printRobotInfo(robot, teamBlue, detection.camera_id());
                }

                //Yellow robot info:
                for (int i = 0; i < robots_yellow_n; i++) {
                    SSL_DetectionRobot robot = detection.robots_yellow(i);
                    printf("-Robot(Y) (%2d/%2d): ",i+1, robots_yellow_n);
                    n=printRobotInfo(robot, teamYellow, detection.camera_id());
                }
                printf("%d robots on the field\n",n);


            }

            //see if packet contains geometry data:
            if (packet.has_geometry()) {
                const SSL_GeometryData & geom = packet.geometry();
                printf("-[Geometry Data]-------\n");

                const SSL_GeometryFieldSize & field = geom.field();
                soccerView->LoadFieldGeometry((SSL_GeometryFieldSize&)field);
                printf("Field Dimensions:\n");
                printf("  -line_width=%d (mm)\n",field.line_width());
                printf("  -field_length=%d (mm)\n",field.field_length());
                printf("  -field_width=%d (mm)\n",field.field_width());
                printf("  -boundary_width=%d (mm)\n",field.boundary_width());
                printf("  -referee_width=%d (mm)\n",field.referee_width());
                printf("  -goal_width=%d (mm)\n",field.goal_width());
                printf("  -goal_depth=%d (mm)\n",field.goal_depth());
                printf("  -goal_wall_width=%d (mm)\n",field.goal_wall_width());
                printf("  -center_circle_radius=%d (mm)\n",field.center_circle_radius());
                printf("  -defense_radius=%d (mm)\n",field.defense_radius());
                printf("  -defense_stretch=%d (mm)\n",field.defense_stretch());
                printf("  -free_kick_from_defense_dist=%d (mm)\n",field.free_kick_from_defense_dist());
                printf("  -penalty_spot_from_field_line_dist=%d (mm)\n",field.penalty_spot_from_field_line_dist());
                printf("  -penalty_line_from_spot_dist=%d (mm)\n",field.penalty_line_from_spot_dist());

                int calib_n = geom.calib_size();
                for (int i=0; i< calib_n; i++) {
                    const SSL_GeometryCameraCalibration & calib = geom.calib(i);
                    printf("Camera Geometry for Camera ID %d:\n", calib.camera_id());
                    printf("  -focal_length=%.2f\n",calib.focal_length());
                    printf("  -principal_point_x=%.2f\n",calib.principal_point_x());
                    printf("  -principal_point_y=%.2f\n",calib.principal_point_y());
                    printf("  -distortion=%.2f\n",calib.distortion());
                    printf("  -q0=%.2f\n",calib.q0());
                    printf("  -q1=%.2f\n",calib.q1());
                    printf("  -q2=%.2f\n",calib.q2());
                    printf("  -q3=%.2f\n",calib.q3());
                    printf("  -tx=%.2f\n",calib.tx());
                    printf("  -ty=%.2f\n",calib.ty());
                    printf("  -tz=%.2f\n",calib.tz());
                }
            }            
        }

    }
}

void ViewUpdateThread::Terminate()
{
    shutdownView = true;
}
