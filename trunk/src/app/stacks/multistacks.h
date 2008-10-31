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
  \file    multistacks.h
  \brief   A collection of useful MultiVisionStacks
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#ifndef CUSTOMMULTISTACKS_H
#define CUSTOMMULTISTACKS_H

#include "multivisionstack.h"
#include "stacks.h"

using namespace std;

/*!
  \class   MultiStackRoboCupSSL
  \brief   The multi-camera vision processing stack used for the RoboCup SSL vision system.
  \author  Stefan Zickler, (C) 2008
*/
class MultiStackRoboCupSSL : public MultiVisionStack {
  protected:

  public:
  MultiStackRoboCupSSL(RenderOptions * _opts, int cameras) : MultiVisionStack("RoboCup SSL Multi-Cam",_opts) {
    //add parameter for number of cameras
    createThreads(cameras);
    unsigned int n = threads.size();
    for (unsigned int i = 0; i < n;i++) {
      threads[i]->setFrameBuffer(new FrameBuffer(5));
      threads[i]->setStack(new VsRoboCup(_opts,threads[i]->getFrameBuffer()));
     }
  }
  virtual string getSettingsFileName() {
    return "robocup-ssl";
  }
  virtual ~MultiStackRoboCupSSL() {
    stop();
  }
};





#endif
