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
  \file    stack_robocup_ssl.cpp
  \brief   C++ Implementation: stack_robocup_ssl
  \author  Author Name, 2008
*/
//========================================================================
#include "stack_robocup_ssl.h"

StackRoboCupSSL::StackRoboCupSSL(RenderOptions * _opts, FrameBuffer * _fb, string cam_settings_filename) : VisionStack("RoboCup Image Processing",_opts) {
    (void)_fb;
    _cam_settings_filename=cam_settings_filename;
    lut_yuv = new YUVLUT(4,6,6,cam_settings_filename + "-lut-yuv.xml");
    lut_yuv->loadRoboCupChannels(LUTChannelMode_Numeric);

    stack.push_back(new PluginColorCalibration(_fb,lut_yuv, LUTChannelMode_Numeric));
    settings->addChild(lut_yuv->getSettings());

    stack.push_back(new PluginColorThreshold(_fb,lut_yuv));

    //initialize the runlength encoder...
    //we don't expect more than 100k runs per image
    stack.push_back(new PluginRunlengthEncode(_fb,50000));

    //initialize the blob finder
    //we don't expect more than 10k blobs per image
    stack.push_back(new PluginFindBlobs(_fb,lut_yuv, 10000));

    stack.push_back(new PluginDetectBalls(_fb,lut_yuv));

    PluginVisualize * vis=new PluginVisualize(_fb);
    vis->setThresholdingLUT(lut_yuv);
    stack.push_back(vis);


}
string StackRoboCupSSL::getSettingsFileName() {
  return _cam_settings_filename;
}
StackRoboCupSSL::~StackRoboCupSSL() {
  delete lut_yuv;
}

