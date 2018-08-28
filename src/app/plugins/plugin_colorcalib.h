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
  \file    plugin_colorcalib.h
  \brief   C++ Interface: plugin_colorcalib
  \author  Author Name, 2008
*/
//========================================================================
#ifndef PLUGIN_COLORCALIB_H
#define PLUGIN_COLORCALIB_H

#include "framedata.h"
#include "VarTypes.h"
#include "visionplugin.h"
#include "lut3d.h"
#include "lutwidget.h"

/**
	@author Stefan Zickler <szickler@cs.cmu.edu>
*/
class PluginColorCalibration : public VisionPlugin
{
Q_OBJECT
protected:
    YUVLUT * lut;
    VarList * settings;
    LUTWidget * lutw;
    VarStringEnum * v_lut_sources;
    VarStringEnum * v_lut_colors;
    VarTrigger * copy_LUT_trigger;
    LUTChannelMode mode;
    bool continuing_undo;
    void mouseEvent ( QMouseEvent * event, pixelloc loc );
    void copyLUT();
protected slots:
    void slotCopyLUT();
public:
    PluginColorCalibration(FrameBuffer * _buffer, YUVLUT * _lut, LUTChannelMode _mode=LUTChannelMode_Numeric);
    virtual VarList * getSettings();
    virtual string getName();
    virtual ~PluginColorCalibration();
    virtual QWidget * getControlWidget();

    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseReleaseEvent ( QMouseEvent * event, pixelloc loc );
    virtual void mouseMoveEvent ( QMouseEvent * event, pixelloc loc );

private:
    void updateColorList();
};

#endif
