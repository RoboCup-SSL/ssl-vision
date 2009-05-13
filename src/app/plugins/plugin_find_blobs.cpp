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

  _settings=new VarList("Blob Finding");
  _settings->addChild(_v_min_blob_area=new VarInt("min_blob_area", 5));
  _settings->addChild(_v_enable=new VarBool("enable", true));

}


PluginFindBlobs::~PluginFindBlobs()
{
}



ProcessResult PluginFindBlobs::process(FrameData * data, RenderOptions * options) {
  (void)options;


  CMVision::RegionList * reglist;
  if ((reglist=(CMVision::RegionList *)data->map.get("cmv_reglist")) == 0) {
    reglist=(CMVision::RegionList *)data->map.insert("cmv_reglist",new CMVision::RegionList(max_regions));
  }

  CMVision::ColorRegionList * colorlist;
  if ((colorlist=(CMVision::ColorRegionList *)data->map.get("cmv_colorlist")) == 0) {
    colorlist=(CMVision::ColorRegionList *)data->map.insert("cmv_colorlist",new CMVision::ColorRegionList(lut->getChannelCount()));
  }

  CMVision::RunList * runlist;
  if ((runlist=(CMVision::RunList *)data->map.get("cmv_runlist")) == 0) {
    printf("Blob finder: no runlength-encoded input list was found!\n");
    return ProcessingFailed;
  }

  if (_v_enable->getBool()==true) {
    //Connect the components of the runlength map:
    CMVision::RegionProcessing::connectComponents(runlist);
  
    //Extract Regions from runlength map:
    CMVision::RegionProcessing::extractRegions(reglist, runlist);
  
    if (reglist->getUsedRegions() == reglist->getMaxRegions()) {
      printf("Warning: extract regions exceeded maximum number of %d regions\n",reglist->getMaxRegions());
    }
  
    //Separate Regions by colors:
    int max_area = CMVision::RegionProcessing::separateRegions(colorlist, reglist, _v_min_blob_area->getInt());
  
    //Sort Regions:
    CMVision::RegionProcessing::sortRegions(colorlist,max_area);
  } else {
    //detect nothing.
    reglist->setUsedRegions(0);
    int num_colors=colorlist->getNumColorRegions();
    CMVision::RegionLinkedList * color=colorlist->getColorRegionArrayPointer();
  
    // clear out the region list head table
    for(int i=0; i<num_colors; i++){
      color[i].reset();
    }
  }

  return ProcessingOk;

}

VarList * PluginFindBlobs::getSettings() {
  return _settings;
}

string PluginFindBlobs::getName() {
  return "FindBlobs";
}
