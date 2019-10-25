#include "plugin_apriltag.h"
#include "messages_robocup_ssl_detection.pb.h"
#include <Eigen/Dense>
#include <apriltag_pose.h>
#include <array>
#include <cstdio>
#include <image.h>
#include <iostream>
#include <tag16h5.h>
#include <tag25h9.h>
#include <tag36h11.h>
#include <tagCircle21h7.h>
#include <tagCircle49h12.h>
#include <tagCustom20h7.h>
#include <tagCustom48h12.h>
#include <tagStandard41h12.h>
#include <tagStandard52h13.h>

using HammHist = std::array<int, 10>;
using Eigen::AngleAxisd;
using Eigen::Isometry3d;
using Eigen::Matrix3d;
using Eigen::Quaterniond;
using Eigen::Vector3d;

enum class Team : int { None, Blue, Yellow };

PluginAprilTag::PluginAprilTag(FrameBuffer *buffer,
                               const CameraParameters &camera_params,
                               std::shared_ptr<TeamTags> blue_team_tags,
                               std::shared_ptr<TeamTags> yellow_team_tags,
                               CMPattern::TeamSelector &blue_team_selector,
                               CMPattern::TeamSelector &yellow_team_selector)
    : VisionPlugin(buffer), settings(new VarList("AprilTag")),
      v_enable(new VarBool("enable", true)),
      v_quad_size(new VarDouble("quad size (mm)", 70.0)),
      v_iters(new VarInt("iters", 1)), v_threads(new VarInt("threads", 1)),
      v_hamming(new VarInt("hamming", 1)),
      v_decimate(new VarDouble("decimate", 2.0)),
      v_blur(new VarDouble("blur", 0.0)),
      v_refine_edges(new VarBool("refine-edges", true)),
      detections{nullptr, &apriltag_detections_destroy},
      camera_params(camera_params), blue_team_tags(blue_team_tags),
      yellow_team_tags(yellow_team_tags),
      blue_team_selector(blue_team_selector),
      yellow_team_selector(yellow_team_selector) {

  v_family.reset(new VarStringEnum("Tag Family", "tagCircle21h7"));
  v_family->addItem("tag36h11");
  v_family->addItem("tag16h5");
  v_family->addItem("tagCircle21h7");
  v_family->addItem("tagCircle49h12");
  v_family->addItem("tagStandard41h12");
  v_family->addItem("tagStandard52h13");
  v_family->addItem("tagCustom48h12");
  v_family->addItem("tagCustom20h7");

  settings->addChild(v_enable.get());
  settings->addChild(v_family.get());
  settings->addChild(v_quad_size.get());
  settings->addChild(v_iters.get());
  settings->addChild(v_threads.get());
  settings->addChild(v_hamming.get());
  settings->addChild(v_decimate.get());
  settings->addChild(v_blur.get());
  settings->addChild(v_refine_edges.get());

  // construct apriltag detector with these settings
  makeTagFamily();
  makeTagDetector();
}

ProcessResult PluginAprilTag::process(FrameData *data, RenderOptions *options) {
  if (!v_enable->getBool()) {
    return ProcessingOk;
  }
  if (!tag_family || v_family->getString() != tag_family->name) {
    makeTagFamily();
    if (!tag_family) {
      std::cerr << "AprilTag: Failed to create tag family '"
                << v_family->getString() << "'\n";
      return ProcessingFailed;
    }

    apriltag_detector_clear_families(tag_detector.get());
    apriltag_detector_add_family_bits(tag_detector.get(), tag_family.get(),
                                      v_hamming->getInt());
  }

  if (!tag_detector) {
    makeTagDetector();
    if (!tag_detector) {
      std::cerr << "AprilTag: Failed to create tag detector\n";
      return ProcessingFailed;
    }
  }

  tag_detector->quad_decimate = v_decimate->getDouble();
  tag_detector->quad_sigma = v_blur->getDouble();
  tag_detector->nthreads = v_threads->getInt();
  tag_detector->refine_edges = v_refine_edges->getBool();

  Image<raw8> *img_greyscale =
      reinterpret_cast<Image<raw8> *>(data->map.get("greyscale"));
  if (img_greyscale == nullptr) {
    std::cerr << "AprilTag: No greyscale image in frame data\n";
    return ProcessingFailed;
  }

  const int maxiters = v_iters->getInt();
  for (int iter = 0; iter < maxiters; iter++) {
    if (maxiters > 1) {
      std::cout << "iter " << iter + 1 << " / " << maxiters << "\n";
    }

    // zero-copy. Just use the already allocated buffer data.
    image_u8_t im{.width = img_greyscale->getWidth(),
                  .height = img_greyscale->getHeight(),
                  .stride = img_greyscale->getWidth(),
                  .buf = img_greyscale->getData()};

    detections.reset(apriltag_detector_detect(tag_detector.get(), &im));
    data->map.update("apriltag_detections", detections.get());

    // add the detections to the robot list so that the positions will
    // be published
    SSL_DetectionFrame *detection_frame = 0;
    detection_frame =
        (SSL_DetectionFrame *)data->map.get("ssl_detection_frame");
    if (detection_frame == 0)
      detection_frame = (SSL_DetectionFrame *)data->map.insert(
          "ssl_detection_frame", new SSL_DetectionFrame());

    // add the detections to the detected robots list
    Eigen::Matrix<double, 3, 4> ideal_tag_points;
    ideal_tag_points << -1, 1, -1, 1, 1, 1, -1, -1, 1, 1, 1, 1;

    for (int i = 0; i < zarray_size(detections.get()); ++i) {
      apriltag_detection_t *det;
      zarray_get(detections.get(), i, &det);

      // check if tag in map
      // if not then ignore it
      Team tag_team = Team::None;
      double team_height;
      if ((*blue_team_tags)->count(det->id)) {
        tag_team = Team::Blue;
        team_height =
            blue_team_selector.getSelectedTeam()->_robot_height->getDouble();
      }
      if ((*yellow_team_tags)->count(det->id)) {
        tag_team = Team::Yellow;
        team_height =
            yellow_team_selector.getSelectedTeam()->_robot_height->getDouble();
      }

      if (tag_team == Team::None) {
        continue;
      }

      Eigen::Map<const Eigen::Matrix3d> tag_homography(det->H->data);

      Eigen::Matrix<double, 3, 4> image_tag_points =
          tag_homography.transpose() * ideal_tag_points;
      image_tag_points.array().rowwise() /= image_tag_points.row(2).array();

      // convert the points to field coordinates
      vector3d p_top_left_field;
      vector2d p_top_left_image(image_tag_points(0, 0), image_tag_points(1, 0));
      camera_params.image2field(p_top_left_field, p_top_left_image,
                                team_height);

      vector3d p_top_right_field;
      vector2d p_top_right_image(image_tag_points(0, 1),
                                 image_tag_points(1, 1));
      camera_params.image2field(p_top_right_field, p_top_right_image,
                                team_height);

      vector3d p_bottom_left_field;
      vector2d p_bottom_left_image(image_tag_points(0, 2),
                                   image_tag_points(1, 2));
      camera_params.image2field(p_bottom_left_field, p_bottom_left_image,
                                team_height);

      vector3d p_bottom_right_field;
      vector2d p_bottom_right_image(image_tag_points(0, 3),
                                    image_tag_points(1, 3));
      camera_params.image2field(p_bottom_right_field, p_bottom_right_image,
                                team_height);

      vector3d p_center_field;
      vector2d p_center_image(det->c[0], det->c[1]);
      camera_params.image2field(p_center_field, p_center_image, team_height);

      // the quad in field coordinates may not be perfect due to
      // detection errors, numerical errors and camera calibration. So
      // get the angles from each face, and average to get a better
      // estimate.
      Eigen::RowVector4d angles = Eigen::RowVector4d::Zero();

      angles(0) = atan2(p_top_right_field.x - p_top_left_field.x,
                        p_top_left_field.y - p_top_right_field.y);

      angles(1) = atan2(p_bottom_right_field.y - p_top_right_field.y,
                        p_bottom_right_field.x - p_top_right_field.x);

      angles(2) = atan2(p_bottom_right_field.x - p_bottom_left_field.x,
                        p_bottom_left_field.y - p_bottom_right_field.y);

      angles(3) = atan2(p_bottom_left_field.y - p_top_left_field.y,
                        p_bottom_left_field.x - p_top_left_field.x);

      SSL_DetectionRobot *robot_detection = nullptr;
      if (tag_team == Team::Blue) {
        robot_detection = detection_frame->add_robots_blue();
      } else {
        robot_detection = detection_frame->add_robots_yellow();
      }
      robot_detection->set_confidence(
          det->decision_margin); // TODO(dschwab): What value should I put here?
      robot_detection->set_robot_id(det->id);

      robot_detection->set_x(p_center_field.x);
      robot_detection->set_y(p_center_field.y);
      robot_detection->set_orientation(angles.mean());
      robot_detection->set_pixel_x(det->c[0]);
      robot_detection->set_pixel_y(det->c[1]);
      robot_detection->set_height(team_height);
    }
  }

  return ProcessingOk;
}

VarList *PluginAprilTag::getSettings() { return settings.get(); }

std::string PluginAprilTag::getName() { return "AprilTag"; }

void PluginAprilTag::makeTagFamily() {
  const auto tag_name = v_family->getString();
  if (tag_name == "tag36h11") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tag36h11_create(), &tag36h11_destroy};
  } else if (tag_name == "tag25h9") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tag25h9_create(), &tag25h9_destroy};
  } else if (tag_name == "tag16h5") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tag16h5_create(), &tag16h5_destroy};
  } else if (tag_name == "tagCircle21h7") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tagCircle21h7_create(), &tagCircle21h7_destroy};
  } else if (tag_name == "tagCircle49h12") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tagCircle49h12_create(), &tagCircle49h12_destroy};
  } else if (tag_name == "tagStandard41h12") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tagStandard41h12_create(), &tagStandard41h12_destroy};
  } else if (tag_name == "tagStandard52h13") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tagStandard52h13_create(), &tagStandard52h13_destroy};
  } else if (tag_name == "tagCustom48h12") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tagCustom48h12_create(), &tagCustom48h12_destroy};
  } else if (tag_name == "tagCustom20h7") {
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        tagCustom20h7_create(), &tagCustom20h7_destroy};
  } else {
    std::cerr << "AprilTag: Failed to create apriltag family. Unknown family '"
              << tag_name << "'\n";
    tag_family = std::unique_ptr<apriltag_family_t,
                                 std::function<void(apriltag_family_t *)>>{
        nullptr, [](apriltag_family_t *) {}};
  }
}

void PluginAprilTag::makeTagDetector() {
  if (!tag_family) {
    std::cerr
        << "AprilTag: Cannot construct tag detector because tag family has not "
           "been created\n";
    tag_detector = std::unique_ptr<apriltag_detector_t,
                                   std::function<void(apriltag_detector_t *)>>{
        nullptr, [](apriltag_detector_t *) {}};
    return;
  }

  tag_detector = std::unique_ptr<apriltag_detector_t,
                                 std::function<void(apriltag_detector_t *)>>{
      apriltag_detector_create(), &apriltag_detector_destroy};

  apriltag_detector_add_family_bits(tag_detector.get(), tag_family.get(),
                                    v_hamming->getInt());
}
