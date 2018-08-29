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
  \file    robocup_ssl_server.h
  \brief   C++ Interface: robocup_ssl_server
  \author  Stefan Zickler, 2009
*/
//========================================================================
#ifndef ROBOCUP_SSL_SERVER_H
#define ROBOCUP_SSL_SERVER_H
#include "netraw.h"
#include <string>
#include <QMutex>
#include "messages_robocup_ssl_detection.pb.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "messages_robocup_ssl_geometry_legacy.pb.h"
#include "messages_robocup_ssl_wrapper.pb.h"
#include "messages_robocup_ssl_wrapper_legacy.pb.h"
using namespace std;
/**
	@author Stefan Zickler
*/
class RoboCupSSLServer{
friend class MultiStackRoboCupSSL;
protected:
  Net::UDP mc; // multicast server
  QMutex mutex;
  int _port;
  string _net_address;
  string _net_interface;

public:
    RoboCupSSLServer(int port,
                     string net_ref_address,
                     string net_ref_interface="");

    ~RoboCupSSLServer();
    bool open();
    void close();
    template <typename T>
    bool sendWrapperPacket(const T & packet) {
      string buffer;
      packet.SerializeToString(&buffer);
      Net::Address multiaddr;
      multiaddr.setHost(_net_address.c_str(),_port);
      bool result;
      result=mc.send(buffer.c_str(),buffer.length(),multiaddr);
      if (result==false) {
        perror("Sendto Error");
        fprintf(stderr,
                "Sending UDP datagram to %s:%d failed (maybe too large?). "
                "Size was: %zu byte(s)\n",
                _net_address.c_str(),
                _port,
                buffer.length());
      }
      return(result);
    }

    bool send(const SSL_DetectionFrame & frame);
    bool send(const SSL_GeometryData & geometry);
    bool sendLegacyMessage(
        const RoboCup2014Legacy::Geometry::SSL_GeometryData & geometry);
    bool sendLegacyMessage(const SSL_DetectionFrame & frame);

};

#endif
