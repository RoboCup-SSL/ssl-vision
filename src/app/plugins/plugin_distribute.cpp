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

#include "plugin_distribute.h"

PluginDistribute::PluginDistribute(FrameBuffer *_buffer, vector<CaptureSplitter *> captureSplitters)
    : VisionPlugin(_buffer),
      captureSplitters(std::move(captureSplitters)) {
  _v_enabled = new VarBool("enable", true);
  _v_image = new VarBool("image", true);
  _v_greyscale = new VarBool("greyscale", false);

  _settings = new VarList("Visualization");
  _settings->addChild(_v_enabled);
  _settings->addChild(_v_image);
  _settings->addChild(_v_greyscale);
}

PluginDistribute::~PluginDistribute() {}

VarList *PluginDistribute::getSettings() { return _settings; }

string PluginDistribute::getName() { return "Visualization"; }

void PluginDistribute::DrawCameraImage(FrameData *data,
                                       VisualizationFrame *vis_frame) {
  // if converting entire image then blanking is not needed
  const ColorFormat source_format = data->video.getColorFormat();
  if (source_format == COLOR_RGB8) {
    // plain copy of data
    memcpy(vis_frame->data.getData(), data->video.getData(),
           data->video.getNumBytes());
  } else if (source_format == COLOR_YUV422_UYVY) {
    Conversions::uyvy2rgb(
        data->video.getData(),
        reinterpret_cast<unsigned char *>(vis_frame->data.getData()),
        data->video.getWidth(), data->video.getHeight());
  } else {
    // blank it:
    vis_frame->data.fillBlack();
    fprintf(stderr, "Unable to visualize color format: %s\n",
            Colors::colorFormatToString(source_format).c_str());
    fprintf(stderr, "Currently supported are rgb8 and yuv422 (UYVY).\n");
    fprintf(stderr, "(Feel free to add more conversions to %s in %s).\n",
            __FUNCTION__, __FILE__);
  }
  if (_v_greyscale->getBool() == true) {
    unsigned int n = vis_frame->data.getNumPixels();
    rgb *vis_ptr = vis_frame->data.getPixelData();
    rgb color;
    for (unsigned int i = 0; i < n; i++) {
      color = vis_ptr[i];
      color.r = color.g = color.b = ((color.r + color.g + color.b) / 3);
      vis_ptr[i] = color;
    }
  }
}

ProcessResult PluginDistribute::process(FrameData *data,
                                        RenderOptions *options) {
  if (data == 0)
    return ProcessingFailed;

  for(int i=0;i<captureSplitters.size();i++) {
    captureSplitters[i]->onNewFrame(&data->video);
  }

  VisualizationFrame *vis_frame =
      reinterpret_cast<VisualizationFrame *>(data->map.get("vis_frame"));
  if (vis_frame == 0) {
    vis_frame = reinterpret_cast<VisualizationFrame *>(
        data->map.insert("vis_frame", new VisualizationFrame()));
  }

  if (_v_enabled->getBool()) {
    // check video data...
    if (data->video.getWidth() == 0 || data->video.getHeight() == 0) {
      // there is no valid video data
      // mark visualization data as invalid
      vis_frame->valid = false;
      return ProcessingOk;
    } else {
      // allocate visualization frame accordingly:
      vis_frame->data.allocate(data->video.getWidth(), data->video.getHeight());
    }

    // Draw camera image
    if (_v_image->getBool()) {
      DrawCameraImage(data, vis_frame);
    } else {
      vis_frame->data.fillBlack();
    }

    vis_frame->valid = true;
  } else {
    vis_frame->valid = false;
  }
  return ProcessingOk;
}