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

ViewUpdateThread::ViewUpdateThread ( SoccerView *_soccerView )
{
  soccerView = _soccerView;
  shutdownView = false;
}

void ViewUpdateThread::run()
{
  int n=0;
  client.open ( true );
  while ( !shutdownView )
  {
    if ( client.receive ( packet ) )
    {
      //see if the packet contains a robot detection frame:
      if ( packet.has_detection() )
      {
        SSL_DetectionFrame detection = packet.detection();
        int balls_n = detection.balls_size();
        //Ball info:
        QVector<QPointF> balls;
        for ( int i = 0; i < balls_n; i++ )
        {
          QPointF p;
          SSL_DetectionBall ball = detection.balls ( i );
          if ( ball.confidence() > 0.0 )
          {
            p.setX ( ball.x() );
            p.setY ( ball.y() );
            balls.push_back ( p );
          }
        }
        soccerView->UpdateBalls ( balls,detection.camera_id() );
        //Robot info:
        soccerView->UpdateRobots ( detection );
      }
      //see if packet contains geometry data:
      if ( packet.has_geometry() )
      {
        const SSL_GeometryData & geom = packet.geometry();
        const SSL_GeometryFieldSize & field = geom.field();
        soccerView->LoadFieldGeometry ( ( SSL_GeometryFieldSize& ) field );
      }
      //soccerView->updateView();
    }
  }
}

void ViewUpdateThread::Terminate()
{
  shutdownView = true;
}
