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
  \file    sslReceiverThread.h
  \brief   Thread for receiving data from SSL-Vision
  \author  Tim Laue (C) 2010
*/
//========================================================================


#ifndef SSL_RECEIVER_THREAD
#define SSL_RECEIVER_THREAD

#include <QThread>
#include "robocup_ssl_client.h"
#include "gClientBlackBoard.h"


class SSLReceiverThread : public QThread
{
 private:
    bool stopReceiving;
    RoboCupSSLClient client;
    SSL_WrapperPacket packet;
    GClientBlackBoard& blackBoard;
    
 public:
    SSLReceiverThread(GClientBlackBoard& blackBoard):stopReceiving(false), blackBoard(blackBoard)
    {
        client.open(true);
    }
    
    void setStopReceiving() 
    {
        stopReceiving=true;
    }
    
    void run()
    {
        while(!stopReceiving)
        {
            if (client.receive(packet)) 
            {
                if(packet.has_detection()) 
                {
                    const SSL_DetectionFrame& detection = packet.detection();
                    blackBoard.setDetection(detection);
                }
                if (packet.has_geometry()) 
                {
                    const SSL_GeometryData & geometry = packet.geometry();
                    blackBoard.setGeometry(geometry);
                }
            }
        }
    }
};

#endif

