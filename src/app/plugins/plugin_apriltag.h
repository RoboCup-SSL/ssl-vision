#ifndef PLUGIN_APRILTAG_H
#define PLUGIN_APRILTAG_H

#include <apriltag.h>
#include <functional>
#include <memory>
#include <string>
#include <visionplugin.h>

class PluginAprilTag : public VisionPlugin {
protected:
  std::unique_ptr<VarList> settings;
  std::unique_ptr<VarBool> v_enable;
  std::unique_ptr<VarStringEnum> v_family;
  std::unique_ptr<VarDouble> v_quad_size;
  std::unique_ptr<VarInt> v_iters;
  std::unique_ptr<VarInt> v_threads;
  std::unique_ptr<VarInt> v_hamming;
  std::unique_ptr<VarDouble> v_decimate;
  std::unique_ptr<VarDouble> v_blur;
  std::unique_ptr<VarBool> v_refine_edges;

  std::unique_ptr<apriltag_family_t, std::function<void(apriltag_family_t *)>>
      tag_family;
  std::unique_ptr<apriltag_detector_t,
                  std::function<void(apriltag_detector_t *)>>
      tag_detector;
  std::unique_ptr<zarray_t, std::function<void(zarray_t *)>> detections;

public:
  PluginAprilTag(FrameBuffer *buffer);
  ~PluginAprilTag() override = default;

  ProcessResult process(FrameData *data, RenderOptions *options) override;

  VarList *getSettings() override;

  std::string getName() override;

private:
  void makeTagFamily();
  void makeTagDetector();
};

#endif /* PLUGIN_APRILTAG_H */
