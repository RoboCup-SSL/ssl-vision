#ifndef PLUGIN_GREYSCALE_H
#define PLUGIN_GREYSCALE_H

#include <image.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <string>
#include <visionplugin.h>

class PluginGreyscale : public VisionPlugin {
protected:
  std::unique_ptr<VarList> settings;

public:
  PluginGreyscale(FrameBuffer *buffer);
  ~PluginGreyscale() override = default;

  ProcessResult process(FrameData *data, RenderOptions *options) override;

  VarList *getSettings() override;

  std::string getName() override;

private:
  void cvColor2Grey(const RawImage& src, const int src_data_format,
                    Image<raw8> *dst,
                    const cv::ColorConversionCodes conversion_code);
  void cv16bit2_8bit(const RawImage& src, Image<raw8> *dst);
  void manualColor2Grey(const RawImage& src, Image<raw8> *dst);
  void copyData(const RawImage& src, Image<raw8> *dst);
};

#endif /* PLUGIN_GREYSCALE_H */
