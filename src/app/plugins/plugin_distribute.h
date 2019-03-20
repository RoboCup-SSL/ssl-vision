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

#pragma once

#include <visionplugin.h>
#include <captureinterface.h>
#include "image.h"
#include "plugin_visualize.h"
#include "capture_splitter.h"

class PluginDistribute : public VisionPlugin {
protected:
  VarList *_settings;
  VarBool *_v_enabled;
  VarBool *_v_image;
  VarBool *_v_greyscale;

  std::vector<CaptureSplitter*> captureSplitters;

  void drawCameraImage(FrameData *data, VisualizationFrame *vis_frame);

public:
  PluginDistribute(FrameBuffer *_buffer, vector<CaptureSplitter *> captureSplitters);

  ~PluginDistribute();

  virtual ProcessResult process(FrameData *data, RenderOptions *options);
  virtual VarList *getSettings();
  virtual string getName();
};
