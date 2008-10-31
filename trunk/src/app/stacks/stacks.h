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
  \file    stacks.h
  \brief   A collection of single-camera vision stacks
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#ifndef CUSTOMVISIONSTACKS_H
#define CUSTOMVISIONSTACKS_H

#include "visionstack.h"
#include "lut3d.h"

using namespace std;

/*!
  \class   VsRoboCup
  \brief   The single camera vision stack implementation used for the RoboCup SSL
  \author  Stefan Zickler, (C) 2008
           multiple of these stacks are run in parallel using the MultiStackRoboCupSSL
*/
class VsRoboCup : public VisionStack {
  protected:
  YUVLUT * lut;
  //VarTsaiCalibrationData * tsai_data;
  public:
  VsRoboCup(RenderOptions * _opts, FrameBuffer * _fb) : VisionStack("RoboCup Image Processing",_opts) {
    (void)_fb;
    lut=new YUVLUT(4,6,6,"calib_lut.xml");
    lut->loadBlackWhite();
    //tsai_data=new VarTsaiCalibrationData("lightstripe_tsai.xml");
    /*stack.push_back(new PluginAdjust(_fb));
    stack.push_back(new PluginColorCalibration(_fb,lut));
    stack.push_back(new PluginColorSegmentation(_fb,lut));
    stack.push_back(new PluginPatternDetect(_fb));
    stack.push_back(new PluginTsaiCal(_fb,tsai_data));
    settings->addChild(lut->getSettings());
    settings->addChild(tsai_data->getSettings());*/
  }
  virtual string getSettingsFileName() {
    return "calib";
  }
  virtual ~VsRoboCup() {
    delete lut;
  }
};


#endif
