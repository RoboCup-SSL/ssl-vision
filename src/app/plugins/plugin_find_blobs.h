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
  \file    plugin_find_blobs.h
  \brief   C++ Interface: plugin_find_blobs
  \author  Author Name, 2008
*/
//========================================================================
#ifndef PLUGIN_FIND_BLOBS_H
#define PLUGIN_FIND_BLOBS_H

#include <visionplugin.h>
#include "lut3d.h"
#include "cmvision_region.h"
/**
	@author Stefan Zickler
*/
class PluginFindBlobs : public VisionPlugin
{
protected:
  YUVLUT * lut;

  VarList * _settings;
  VarInt * _v_min_blob_area;
  VarBool * _v_enable;
  VarInt * v_max_regions;
public:
    PluginFindBlobs(FrameBuffer * _buffer, YUVLUT * _lut);

    ~PluginFindBlobs() override;

    ProcessResult process(FrameData * data, RenderOptions * options) override;

    VarList * getSettings() override;

    string getName() override;
};

#endif
