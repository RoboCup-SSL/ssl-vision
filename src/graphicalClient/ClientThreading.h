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
  \file    ClientThreading.h
  \brief   C++ Interface: ViewUpdateThread
  \author  Joydeep Biswas, Stefan Zickler (C) 2009
*/
//========================================================================

#ifndef VIEWUPDATETHREAD_H
#define VIEWUPDATETHREAD_H

#include <QThread>
#include <QVector>
#include <QPointF>
#include "GraphicsPrimitives.h"
#include "robocup_ssl_client.h"
#include "timer.h"

class ViewUpdateThread : public QThread
{
  private:
    bool shutdownView;
    RoboCupSSLClient client;
    SSL_WrapperPacket packet;
    SoccerView *soccerView;

  public:
    ViewUpdateThread() {}
    ViewUpdateThread ( SoccerView *_soccer );
    void run();
    void Terminate();
};

#endif // VIEWUPDATETHREAD_H
