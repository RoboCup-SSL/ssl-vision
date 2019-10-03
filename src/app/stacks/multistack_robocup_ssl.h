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
  \file    multistack_robocup_ssl.h
  \brief   C++ Interface: multistack_robocup_ssl
  \author  Author Name, 2008
*/
//========================================================================
#ifndef MULTISTACK_ROBOCUP_SSL_H
#define MULTISTACK_ROBOCUP_SSL_H

#include <QObject>
#include "multivisionstack.h"
#include "stack_robocup_ssl.h"
#include "plugin_detect_balls.h"
#include "plugin_publishgeometry.h"
#include "cmpattern_teamdetector.h"
#include "robocup_ssl_server.h"
#include "field.h"
using namespace std;

/*!
  \class   MultiStackRoboCupSSL
  \brief   The multi-camera vision processing stack used for the RoboCup SSL vision system.
  \author  Stefan Zickler, (C) 2008
*/
class MultiStackRoboCupSSL : public QObject, public MultiVisionStack {
  Q_OBJECT
  protected:
  RoboCupField * global_field;
  PluginDetectBallsSettings * global_ball_settings;
  PluginPublishGeometry * global_plugin_publish_geometry;
  PluginLegacyPublishGeometry * legacy_plugin_publish_geometry;
  CMPattern::TeamDetectorSettings * global_team_settings;
  CMPattern::TeamSelector * global_team_selector_blue;
  CMPattern::TeamSelector * global_team_selector_yellow;
  PluginSSLNetworkOutputSettings * global_network_output_settings;
  PluginLegacySSLNetworkOutputSettings * legacy_network_output_settings;

  // UDP Server for Double-Sized field, new protobuf format.
  RoboCupSSLServer * ds_udp_server_new;
  // UDP Server for Double-Sized field, old protobuf format.
  RoboCupSSLServer * ds_udp_server_old;
  public:
  MultiStackRoboCupSSL(RenderOptions *_opts, int num_normal_camera_threads);
  virtual string getSettingsFileName();
  virtual ~MultiStackRoboCupSSL();
  public slots:
  void RefreshNetworkOutput();
  void RefreshLegacyNetworkOutput();
  private:
  void UpdateServerSettings(const int port,
                            const string& address,
                            const string& interface,
                            const string& server_name,
                            RoboCupSSLServer* server);
};

#endif
