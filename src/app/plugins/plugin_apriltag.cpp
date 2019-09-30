#include "plugin_apriltag.h"
#include <array>
#include <cstdio>
#include <image.h>
#include <iostream>
#include <tag16h5.h>
#include <tag25h9.h>
#include <tag36h11.h>
#include <tagCircle21h7.h>
#include <tagCircle49h12.h>
#include <tagCustom48h12.h>
#include <tagStandard41h12.h>
#include <tagStandard52h13.h>

using HammHist = std::array<int, 10>;

PluginAprilTag::PluginAprilTag(FrameBuffer *buffer)
    : VisionPlugin(buffer), settings(new VarList("AprilTag")),
      v_enable(new VarBool("enable", true)),
      v_quad_size(new VarDouble("quad size (mm)", 70.0)),
      v_iters(new VarInt("iters", 1)), v_threads(new VarInt("threads", 1)),
      v_hamming(new VarInt("hamming", 1)),
      v_decimate(new VarDouble("decimate", 2.0)),
      v_blur(new VarDouble("blur", 0.0)),
      v_refine_edges(new VarBool("refine-edges", true)),
      detections{nullptr, &apriltag_detections_destroy} {

  v_family.reset(new VarStringEnum("Tag Family", "tagCircle21h7"));
  v_family->addItem("tag36h11");
  v_family->addItem("tag16h5");
  v_family->addItem("tagCircle21h7");
  v_family->addItem("tagCircle49h12");
  v_family->addItem("tagStandard41h12");
  v_family->addItem("tagStandard52h13");
  v_family->addItem("tagCustom48h12");

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

  // TODO(dschwab): Add option for debug output
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

    // std::cout << "Found " << zarray_size(detections.get()) << "
    // detections\n"; for (int i = 0; i < zarray_size(detections.get()); ++i) {
    //   apriltag_detection_t *det;
    //   zarray_get(detections.get(), i, &det);

    //   printf("detection %3d: id (%2dx%2d)-%-4d, hamming %d, margin %8.3f\n",
    //   i,
    //          det->family->nbits, det->family->h, det->id, det->hamming,
    //          det->decision_margin);
    // }
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
