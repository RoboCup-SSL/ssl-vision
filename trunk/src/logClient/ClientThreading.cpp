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
  \edit    Ulfert Nehmiz (LogPlayer included) 2009
*/
//========================================================================

#include "ClientThreading.h"
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QDateTime>

ViewUpdateThread::ViewUpdateThread ( SoccerView *_soccerView, QMutex* _drawMutex )
{
  soccerView = _soccerView;
  drawMutex = _drawMutex;
  log_control = new LogControl();
  connect(this->soccerView->playLogfile, SIGNAL(pressed()), this, SLOT(playLogfilePressed()) );
  connect(this, SIGNAL(change_play_button(QString)), this->soccerView, SLOT(change_play_button(QString)));
  shutdownView = false;
  play = false;
  fileName = QDir::homePath();
}

void ViewUpdateThread::run()
{
  client.open ( false );
  while ( !shutdownView )
  {
    int time = execute();
    drawMutex->lock();
    soccerView->updateView();
    drawMutex->unlock();
    msleep(abs(time));
  }
}

int ViewUpdateThread::execute()
{
    int c = 0;
    int n=0;
    Log_Frame* log_frame;
    SSL_DetectionFrame detection;
    if ( client.receive ( packet ) && !play)
    {
      //see if the packet contains a robot detection frame:
      if ( packet.has_detection() )
      {
        detection = packet.detection();
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
        drawMutex->lock();
        soccerView->UpdateBalls ( balls,detection.camera_id() );
        //Robot info:
        soccerView->UpdateRobots ( detection );
        drawMutex->unlock();
      }
      //see if packet contains geometry data:
      if ( packet.has_geometry() )
      {
        drawMutex->lock();
        const SSL_GeometryData & geom = packet.geometry();
        const SSL_GeometryFieldSize & field = geom.field();
        soccerView->LoadFieldGeometry ( ( SSL_GeometryFieldSize& ) field );
        drawMutex->unlock();
      }
    }

    if(!play && log_control->get_current_frame() != 0)
        end_play_record();

    //Play logfile
    if(play)
    {
        if(log_control->get_next_frame() < 0)
            end_play_record();
        else
        {
            //get next frame
            log_frame = logs.mutable_log(log_control->get_current_frame());
            SSL_DetectionFrame* det_frame = log_frame->mutable_frame();
            detection = *det_frame;

            //process frame
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
            drawMutex->lock();
            soccerView->UpdateBalls ( balls,detection.camera_id() );
            //Robot info:
            soccerView->UpdateRobots ( detection );
            drawMutex->unlock();
        }

        emit update_frame(log_control->get_current_frame());

        //distance between frames in ms
        if(log_control->get_play_speed() == 0)
            return 10;

        //calculate distance between frames
        if(!(log_control->get_prop_next_frame() < 0))
        {
            double old_time = detection.t_capture();
            double new_time = logs.mutable_log(log_control->get_prop_next_frame())->mutable_frame()->t_capture();
            double timediff = new_time - old_time;
            return ((timediff * 1000) / log_control->get_play_speed());
        }

    }

    //distance between frames in ms, see http://xkcd.com/221/
    return 4;
}

void ViewUpdateThread::Terminate()
{
  shutdownView = true;
}

void ViewUpdateThread::playLogfilePressed()
{
    if(!play)
    {
        std::cout << "Start Play Record" << std::endl;
        if(!start_play_record())
        {
            play = true;
        }
    }
    else
    {
        end_play_record();
    }
}

int ViewUpdateThread::start_play_record()
{
    //change fileName into directory
    if(fileName != QDir::homePath())
    {
        int last_slash = 0;
        for(int i = 0; i < fileName.length(); i++)
        {
            if(fileName.at(i).toAscii() == (QChar('/')))
                last_slash = i;
        }
        fileName.remove(last_slash, fileName.length()-last_slash);
    }

    //What data shall I read?
    fileName = QFileDialog::getOpenFileName((QWidget*)this->parent(), tr("Open Logfile"), fileName, tr("Log Files (*.log)"));
    std::cout << "fileName: " << fileName.toAscii().constData() << std::endl;

    // Read the existing log.
    std::fstream input(fileName.toAscii().constData(), std::ios::in | std::ios::binary);
    if (!input)
    {
        std::cout << fileName.toAscii().constData() << ": File not found." << std::endl;
        return -1;
    }
    else if (!logs.ParseFromIstream(&input))
    {
        std::cout << "Failed to parse Logfile." << std::endl;
        return -1;
    }

    std::cout << "File successfully loaded" << std::endl;
    if(logs.IsInitialized() && logs.log_size() > 0 && logs.log(0).IsInitialized())
    {
        std::cout << "Logfilegröße:  " << logs.log_size() << std::endl;
        std::cout << "Start Command: " << logs.mutable_log(0)->refbox_cmd() << std::endl;
        log_control->reset(logs.log_size());
        emit log_size(logs.log_size());
        //initializeSlider(int min, int max, int singleStep, int pageStep, int tickInterval)
        emit initializeSlider(0, logs.log_size(), 1, 100, 1800);
        emit showLogControl(true);
    }
    else
    {
        std::cout << "Logfile seems to be empty or damaged" << std::endl;
        return -1;
    }
    input.close();
    emit change_play_button("  End Play  ");
    return 0;
}

void ViewUpdateThread::end_play_record()
{
    play = false;
    log_control->reset(0);
    logs.clear_log();
    emit showLogControl(false);
    emit change_play_button("Play Record");
    std::cout << "Stopped Playing Record" << std::endl;
}

