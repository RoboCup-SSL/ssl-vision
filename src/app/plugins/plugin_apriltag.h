#ifndef PLUGIN_APRILTAG_H
#define PLUGIN_APRILTAG_H

#include "camera_calibration.h"
#include "cmpattern_teamdetector.h"
#include "team_tags.h"
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

  // april tags info
  std::unique_ptr<apriltag_family_t, std::function<void(apriltag_family_t *)>>
      tag_family;
  std::unique_ptr<apriltag_detector_t,
                  std::function<void(apriltag_detector_t *)>>
      tag_detector;
  std::unique_ptr<zarray_t, std::function<void(zarray_t *)>> detections;

  const CameraParameters &camera_params;
  const std::shared_ptr<TeamTags> blue_team_tags;
  const std::shared_ptr<TeamTags> yellow_team_tags;
  CMPattern::TeamSelector &blue_team_selector;
  CMPattern::TeamSelector &yellow_team_selector;

public:
  PluginAprilTag(FrameBuffer *buffer, const CameraParameters &camera_params,
                 std::shared_ptr<TeamTags> blue_team_tags,
                 std::shared_ptr<TeamTags> yellow_team_tags,
                 CMPattern::TeamSelector &blue_team_selector,
                 CMPattern::TeamSelector &yellow_team_selector);
  ~PluginAprilTag() override = default;

  ProcessResult process(FrameData *data, RenderOptions *options) override;

  VarList *getSettings() override;

  std::string getName() override;

private:
  void makeTagFamily();
  void makeTagDetector();
};

#endif /* PLUGIN_APRILTAG_H */
