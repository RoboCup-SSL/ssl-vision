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
  \file    plugin_runlength_encode.h
  \brief   C++ Interface: plugin_runlength_encode
  \author  Stefan Zickler, 2008
*/
//========================================================================
#ifndef PLUGIN_RUNLENGTHENCODE_H
#define PLUGIN_RUNLENGTHENCODE_H

#include <visionplugin.h>
#include "cmvision_region.h"
#include "timer.h"

/**
	@author Stefan Zickler
*/
class PluginRunlengthEncode : public VisionPlugin
{
protected:
  int _max_runs;
public:
    PluginRunlengthEncode(FrameBuffer * _buffer, int max_runs);

    ~PluginRunlengthEncode();

    virtual ProcessResult process(FrameData * data, RenderOptions * options);

    virtual VarList * getSettings();

    virtual string getName();
};

#endif
