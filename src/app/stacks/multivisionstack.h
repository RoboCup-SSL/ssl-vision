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
  \file    multivisionstack.h
  \brief   C++ Interface: MultiVisionStack
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#ifndef MULTIVISIONSTACK_H
#define MULTIVISIONSTACK_H
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMutex>
#include <QObject>
#include <string>

#include "VarTypes.h"
#include "framedata.h"
#include "realtimedisplaywidget.h"
#include "pixelloc.h"
#include "capture_thread.h"


/*!
  \class   MultiVisionStack
  \brief   Base-class of a multi-threaded / multi-camera vision stack.
  \author  Stefan Zickler, (C) 2008
*/
class MultiVisionStack{
public:
  string name;
  RenderOptions * opts;
  VarList * settings;
protected:
    void createThreads(int number, int max_cameras);
public:
    MultiVisionStack(string _name, RenderOptions * _opts);
    virtual ~MultiVisionStack();
    vector<CaptureThread *> threads;

    VarList * getSettings();
    virtual string getName();
    virtual string getSettingsFileName();

    void start();
    void stop();

    /*virtual void keyPressEvent ( QKeyEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseReleaseEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseMoveEvent ( QMouseEvent * event, pixelloc loc );
    virtual void wheelEvent ( QWheelEvent * event, pixelloc loc );*/

};

#endif
