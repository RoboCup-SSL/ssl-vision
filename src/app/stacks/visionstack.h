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
  \file    visionstack.h
  \brief   C++ Interface: VisionStack
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#ifndef VISIONSTACK_H
#define VISIONSTACK_H

#include "visionplugin.h"
#include "framedata.h"
#include "timer.h"
using namespace std;

/*!
  \class   VisionStack
  \brief   Base-class of a single-threaded / single-camera vision stack.
  \author  Stefan Zickler, (C) 2008
*/
class VisionStack {
protected:
  RenderOptions * opts;
  VarList * settings;
  VarBool * _v_print_timings;
public:
    VisionStack(RenderOptions * _opts);
    virtual ~VisionStack();
    vector<VisionPlugin *> stack;

    VarList * getSettings();

    void process(FrameData * data);
    void postProcess(FrameData * data);
    void updateTimingStatistics();

    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseReleaseEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseMoveEvent ( QMouseEvent * event, pixelloc loc );
    virtual void wheelEvent ( QWheelEvent * event, pixelloc loc );
};

#endif
