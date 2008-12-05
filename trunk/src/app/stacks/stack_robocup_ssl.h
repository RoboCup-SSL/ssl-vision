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
  \file    stack_robocup_ssl.h
  \brief   C++ Interface: stack_robocup_ssl
  \author  Author Name, 2008
*/
//========================================================================
#ifndef STACK_ROBOCUP_SSL_H
#define STACK_ROBOCUP_SSL_H

#include "visionstack.h"
#include "lut3d.h"
#include "plugin_colorcalib.h"
#include "plugin_visualize.h"
#include "plugin_colorthreshold.h"

using namespace std;

/*!
  \class   StackRoboCupSSL
  \brief   The single camera vision stack implementation used for the RoboCup SSL
  \author  Stefan Zickler, (C) 2008
           multiple of these stacks are run in parallel using the MultiStackRoboCupSSL
*/
class StackRoboCupSSL : public VisionStack {
  protected:
  YUVLUT * lut_yuv;
  string _cam_settings_filename;
  public:
  StackRoboCupSSL(RenderOptions * _opts, FrameBuffer * _fb, string cam_settings_filename);
  virtual string getSettingsFileName();
  virtual ~StackRoboCupSSL();
};


#endif
