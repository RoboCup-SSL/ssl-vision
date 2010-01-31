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
  \file    gClientBlackBoard.h
  \brief   Storage for information exchanged by threads
  \author  Tim Laue (C) 2010
*/
//========================================================================


#ifndef G_CLIENT_BLACK_BOARD_H
#define G_CLIENT_BLACK_BOARD_H

#include <QMutex>
#include "messages_robocup_ssl_detection.pb.h"
#include "messages_robocup_ssl_geometry.pb.h"


class GClientBlackBoard 
{
private:
    SSL_DetectionFrame detection;
    SSL_GeometryData geometry;
    bool detectionIsNew;
    bool geometryIsNew;
    QMutex detectionMutex;
    QMutex geometryMutex;
    
 public:
    GClientBlackBoard():detectionIsNew(false), geometryIsNew(false)
    {}
   
    bool isDetectionNew() {return detectionIsNew;} // Should work without mutex
    bool isGeometryNew() {return geometryIsNew;}   // Should work without mutex

    int setDetection(const SSL_DetectionFrame& detection) 
    {
        detectionMutex.lock();
        this->detection = detection;
        detectionIsNew = true;
        detectionMutex.unlock();
    }

    int setGeometry(const SSL_GeometryData& geometry) 
    {
        geometryMutex.lock();
        this->geometry = geometry;
        geometryIsNew = true;
        geometryMutex.unlock();
    }
    
    void getDetection(SSL_DetectionFrame& detection)
    {
        detectionMutex.lock();
        detection = this->detection;
        detectionIsNew = false;
        detectionMutex.unlock();
    }
    
    void getGeometry(SSL_GeometryData& geometry)
    {
        geometryMutex.lock();
        geometry = this->geometry;
        geometryIsNew = false;
        geometryMutex.unlock();
    }
};

#endif

