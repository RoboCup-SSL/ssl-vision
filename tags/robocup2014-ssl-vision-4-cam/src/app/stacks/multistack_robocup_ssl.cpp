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
  \file    multistack_robocup_ssl.cpp
  \brief   C++ Implementation: multistack_robocup_ssl
  \author  Author Name, 2008
*/
//========================================================================
#include "multistack_robocup_ssl.h"

MultiStackRoboCupSSL::MultiStackRoboCupSSL(RenderOptions * _opts, int cameras) : 
    p_ss_udp_server(NULL),
    ds_udp_server_new(NULL), 
    s_ss_udp_server(NULL), 
    ds_udp_server_old(NULL), 
    MultiVisionStack("RoboCup SSL Multi-Cam",_opts) {
  //add global field calibration parameter
  global_field = new RoboCupField();
  settings->addChild(global_field->getSettings());

  global_ball_settings = new PluginDetectBallsSettings();
  settings->addChild(global_ball_settings->getSettings());

  global_team_settings = new CMPattern::TeamDetectorSettings("robocup-ssl-teams.xml");
  settings->addChild(global_team_settings->getSettings());

  global_team_selector_blue = new CMPattern::TeamSelector("Blue Team", global_team_settings);
  settings->addChild(global_team_selector_blue->getSettings());

  global_team_selector_yellow = new CMPattern::TeamSelector("Yellow Team", global_team_settings);
  settings->addChild(global_team_selector_yellow->getSettings());

  global_network_output_settings = new PluginSSLNetworkOutputSettings();
  settings->addChild(global_network_output_settings->getSettings());
  connect(global_network_output_settings->multicast_port,
          SIGNAL(wasEdited(VarType *)),
          this,
          SLOT(RefreshNetworkOutput()));
  connect(global_network_output_settings->multicast_address,
          SIGNAL(wasEdited(VarType *)),
          this,
          SLOT(RefreshNetworkOutput()));
  connect(global_network_output_settings->multicast_interface,
          SIGNAL(wasEdited(VarType *)),
          this,
          SLOT(RefreshNetworkOutput()));

  legacy_network_output_settings = new PluginLegacySSLNetworkOutputSettings();
  settings->addChild(legacy_network_output_settings->getSettings());
  connect(legacy_network_output_settings->p_ss_multicast_port,
          SIGNAL(wasEdited(VarType *)),
          this,
          SLOT(RefreshLegacyNetworkOutput()));
  connect(legacy_network_output_settings->s_ss_multicast_port,
          SIGNAL(wasEdited(VarType *)),
          this,
          SLOT(RefreshLegacyNetworkOutput()));
  connect(legacy_network_output_settings->ds_multicast_port_old,
          SIGNAL(wasEdited(VarType *)),
          this,
          SLOT(RefreshLegacyNetworkOutput()));
  connect(legacy_network_output_settings->multicast_address,
          SIGNAL(wasEdited(VarType *)),
          this,
          SLOT(RefreshLegacyNetworkOutput()));
  connect(legacy_network_output_settings->multicast_interface,
          SIGNAL(wasEdited(VarType *)),
          this,
          SLOT(RefreshLegacyNetworkOutput()));

  p_ss_udp_server = new RoboCupSSLServer(10002, "224.5.23.2");
  ds_udp_server_new = new RoboCupSSLServer(10006, "224.5.23.2");
  s_ss_udp_server = new RoboCupSSLServer(10004, "224.5.23.2");
  ds_udp_server_old = new RoboCupSSLServer(10005, "224.5.23.2");

  global_plugin_publish_geometry = new  PluginPublishGeometry(
      0,
      ds_udp_server_new,
      *global_field);

  legacy_plugin_publish_geometry = new PluginLegacyPublishGeometry (
      0,
      p_ss_udp_server,
      s_ss_udp_server,
      ds_udp_server_old,
      *global_field);

  //add parameter for number of cameras
  createThreads(cameras);
  unsigned int n = threads.size();
  for (unsigned int i = 0; i < n;i++) {
    threads[i]->setFrameBuffer(new FrameBuffer(5));
    threads[i]->setStack(
        new StackRoboCupSSL(
            _opts,threads[i]->getFrameBuffer(),
            i,
            global_field,
            global_ball_settings,
            global_plugin_publish_geometry,
            legacy_plugin_publish_geometry,
            global_team_selector_blue, 
            global_team_selector_yellow,
            p_ss_udp_server,
            ds_udp_server_new,
            s_ss_udp_server,
            ds_udp_server_old,
            "robocup-ssl-cam-" + QString::number(i).toStdString()));
  }
  //TODO: make LUT widgets aware of each other for easy data-sharing
}

string MultiStackRoboCupSSL::getSettingsFileName() {
  return "robocup-ssl";
}

MultiStackRoboCupSSL::~MultiStackRoboCupSSL() {
  stop();
  delete p_ss_udp_server;
  delete ds_udp_server_new;
  delete s_ss_udp_server;
  delete ds_udp_server_old;
  delete global_plugin_publish_geometry;
  delete global_field;
  delete global_ball_settings;
}

void MultiStackRoboCupSSL::UpdateServerSettings(const int port,
                          const string& address,
                          const string& interface,
                          const string& server_name,
                          RoboCupSSLServer* server) {
  server->mutex.lock();
  server->close();
  server->_port = port;
  server->_net_address = address;
  server->_net_interface = interface;
  if (server->open()==false) {
    fprintf(stderr,
            "ERROR WHEN TRYING TO OPEN UDP NETWORK SERVER FOR %s!\n",
            server_name.c_str());
    fflush(stderr);
  }
  server->mutex.unlock();
}

void MultiStackRoboCupSSL::RefreshLegacyNetworkOutput() {
  const string address = 
      legacy_network_output_settings->multicast_address->getString();
  const string interface = 
      legacy_network_output_settings->multicast_interface->getString();

  UpdateServerSettings(
      legacy_network_output_settings->p_ss_multicast_port->getInt(),
      address,
      interface,
      "PRIMARY SINGLE-SIZE FIELD",
      p_ss_udp_server
  );
  UpdateServerSettings(
      legacy_network_output_settings->s_ss_multicast_port->getInt(),
      address,
      interface,
      "SECONDARY SINGLE-SIZE FIELD",
      s_ss_udp_server
  );
  UpdateServerSettings(
      legacy_network_output_settings->ds_multicast_port_old->getInt(),
      address,
      interface,
      "DOUBLE-SIZE FIELD (OLD FORMAT)",
      ds_udp_server_old
  );
}

void MultiStackRoboCupSSL::RefreshNetworkOutput()
{
  UpdateServerSettings(
      global_network_output_settings->multicast_port->getInt(),
      global_network_output_settings->multicast_address->getString(),
      global_network_output_settings->multicast_interface->getString(),
      "DOUBLE-SIZE FIELD (NEW FORMAT)",
      ds_udp_server_new
  );
}