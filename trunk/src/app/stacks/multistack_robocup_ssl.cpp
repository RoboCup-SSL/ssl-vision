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

MultiStackRoboCupSSL::MultiStackRoboCupSSL(RenderOptions * _opts, int cameras) : MultiVisionStack("RoboCup SSL Multi-Cam",_opts) {
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
  connect(global_network_output_settings->multicast_port,SIGNAL(wasEdited(VarType *)),this,SLOT(RefreshNetworkOutput()));
  connect(global_network_output_settings->multicast_address,SIGNAL(wasEdited(VarType *)),this,SLOT(RefreshNetworkOutput()));
  connect(global_network_output_settings->multicast_interface,SIGNAL(wasEdited(VarType *)),this,SLOT(RefreshNetworkOutput()));

  udp_server = new RoboCupSSLServer();

  global_plugin_publish_geometry = new PluginPublishGeometry(0,udp_server,*global_field);

  //add parameter for number of cameras
  createThreads(cameras);
  unsigned int n = threads.size();
  for (unsigned int i = 0; i < n;i++) {
    threads[i]->setFrameBuffer(new FrameBuffer(5));
    threads[i]->setStack(new StackRoboCupSSL(_opts,threads[i]->getFrameBuffer(),i,global_field,global_ball_settings,global_plugin_publish_geometry,global_team_selector_blue, global_team_selector_yellow,udp_server,"robocup-ssl-cam-" + QString::number(i).toStdString()));
  }
    //TODO: make LUT widgets aware of each other for easy data-sharing
}

string MultiStackRoboCupSSL::getSettingsFileName() {
  return "robocup-ssl";
}

MultiStackRoboCupSSL::~MultiStackRoboCupSSL() {
  stop();
  delete udp_server;
  delete global_plugin_publish_geometry;
  delete global_field;
  delete global_ball_settings;
}

void MultiStackRoboCupSSL::RefreshNetworkOutput()
{
  udp_server->mutex.lock();
  udp_server->close();
  udp_server->_port = global_network_output_settings->multicast_port->getInt();
  udp_server->_net_address = global_network_output_settings->multicast_address->getString();
  udp_server->_net_interface = global_network_output_settings->multicast_interface->getString();
  if (udp_server->open()==false) {
    fprintf(stderr,"ERROR WHEN TRYING TO OPEN UDP NETWORK SERVER!\n");
    fflush(stderr);
  }
  udp_server->mutex.unlock();
}
