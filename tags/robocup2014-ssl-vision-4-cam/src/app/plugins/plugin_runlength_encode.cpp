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
  \file    plugin_runlength_encode.cpp
  \brief   C++ Implementation: plugin_runlength_encode
  \author  Stefan Zickler, 2008
*/
//========================================================================
#include "plugin_runlength_encode.h"

PluginRunlengthEncode::PluginRunlengthEncode(FrameBuffer * _buffer, int max_runs)
 : VisionPlugin(_buffer)
{
  _max_runs=max_runs;
}


PluginRunlengthEncode::~PluginRunlengthEncode()
{
}



ProcessResult PluginRunlengthEncode::process(FrameData * data, RenderOptions * options) {
  (void)options;

  CMVision::RunList * runlist;
  if ((runlist=(CMVision::RunList *)data->map.get("cmv_runlist")) == 0) {
    runlist=(CMVision::RunList *)data->map.insert("cmv_runlist",new CMVision::RunList(_max_runs));
  }

  Image<raw8> * img_thresholded = 0;
  if ((img_thresholded=(Image<raw8> *)data->map.get("cmv_threshold")) == 0) {
    printf("Runlength encoder: no thresholded input image found!\n");
    return ProcessingFailed;
  }

  //Runlength Encode the image:
  CMVision::RegionProcessing::encodeRuns(img_thresholded, runlist);
  if (runlist->getUsedRuns() == runlist->getMaxRuns()) {
    printf("Warning: runlength encoder exceeded current max run size of %d\n",runlist->getMaxRuns());
  }

  return ProcessingOk;

}

VarList * PluginRunlengthEncode::getSettings() {
  return 0;
}

string PluginRunlengthEncode::getName() {
  return "RunlengthEncode";
}
