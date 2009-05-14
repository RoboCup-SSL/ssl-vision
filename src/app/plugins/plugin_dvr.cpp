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
  \file    plugin_dvr.cpp
  \brief   C++ Implementation: plugin_dvr
  \author  Stefan Zickler, 2009
*/
//========================================================================
#include "plugin_dvr.h"

PluginDVR::PluginDVR(FrameBuffer * fb)
 : VisionPlugin(fb)
{
 _settings = new VarList("DVR Settings");
}

PluginDVR::~PluginDVR()
{
  delete _settings;
}

VarList * PluginDVR::getSettings() {
  return _settings;
}

string PluginDVR::getName() {
  return "DVR";
}

ProcessResult PluginDVR::process(FrameData * data, RenderOptions * options) {
  (void)data;
  (void)options;

  return ProcessingOk;
}
