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
  \file    plugin_find_blobs.cpp
  \brief   C++ Implementation: plugin_find_blobs
  \author  Stefan Zickler, 2008
*/
//========================================================================
#include "plugin_find_blobs.h"

PluginFindBlobs::PluginFindBlobs(FrameBuffer * _buffer, YUVLUT * _lut, int _max_regions)
 : VisionPlugin(_buffer)
{
  lut=_lut;
  max_regions=_max_regions;
}


PluginFindBlobs::~PluginFindBlobs()
{
}



ProcessResult PluginFindBlobs::process(FrameData * data, RenderOptions * options) {
  (void)options;


  CMVision::Regionlist * reglist;
  if ((reglist=(CMVision::Regionlist *)data->map.get("cmv_reglist")) == 0) {
    reglist=(CMVision::Regionlist *)data->map.insert("cmv_reglist",new CMVision::Regionlist(max_regions));
  }

  CMVision::Runlist * runlist;
  if ((runlist=(CMVision::Runlist *)data->map.get("cmv_runlist")) == 0) {
    printf("Blob finder: no runlength-encoded input list was found!\n");
    return ProcessingFailed;
  }

  //Connect the components of the runlength map:
  CMVisionRegion::connectComponents(runlist);

  //Extract Regions from runlength map:
  CMVisionRegion::extractRegions(reglist, runlist);

  return ProcessingOk;
  
  

}

VarList * PluginFindBlobs::getSettings() {
  return 0;
}

string PluginFindBlobs::getName() {
  return "FindBlobs";
}
