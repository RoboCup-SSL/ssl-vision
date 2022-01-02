#include "plugin_camera_intrinsic_calib.h"

#include <dirent.h>

#include <chrono>
#include <iostream>

#include "conversions_greyscale.h"

PluginCameraIntrinsicCalibration::PluginCameraIntrinsicCalibration(FrameBuffer *buffer,
                                                                   CameraParameters &_camera_params)
    : VisionPlugin(buffer),
      settings(new VarList("Camera Intrinsic Calibration")),
      widget(new CameraIntrinsicCalibrationWidget(_camera_params)),
      camera_params(_camera_params) {
  worker = new PluginCameraIntrinsicCalibrationWorker(_camera_params, widget);

  chessboard_capture_dt = new VarDouble("chessboard capture dT", 0.2);

  auto corner_sub_pixel_list = new VarList("corner sub pixel detection");
  corner_sub_pixel_list->addChild(worker->corner_sub_pixel_windows_size);
  corner_sub_pixel_list->addChild(worker->corner_sub_pixel_max_iterations);
  corner_sub_pixel_list->addChild(worker->corner_sub_pixel_epsilon);

  settings->addChild(worker->image_storage->image_dir);
  settings->addChild(worker->reduced_image_width);
  settings->addChild(chessboard_capture_dt);
  settings->addChild(worker->corner_diff_sq_threshold);
  settings->addChild(corner_sub_pixel_list);
  settings->addChild(worker->fixFocalLength);
  settings->addChild(worker->fixPrinciplePoint);
  settings->addChild(worker->fixTangentialDistortion);
  settings->addChild(worker->fixK1);
  settings->addChild(worker->fixK2);
  settings->addChild(worker->fixK3);
  settings->addChild(worker->useIntrinsicGuess);

  connect(this, SIGNAL(startLoadImages()), worker, SLOT(loadImages()));
  connect(this, SIGNAL(startCalibration()), worker, SLOT(calibrate()));
  connect(this, SIGNAL(startSaveImages()), worker->image_storage, SLOT(saveImages()));
}

PluginCameraIntrinsicCalibration::~PluginCameraIntrinsicCalibration() {
  worker->deleteLater();
  delete widget;
  delete worker;
  delete chessboard_capture_dt;
}

VarList *PluginCameraIntrinsicCalibration::getSettings() { return settings.get(); }

std::string PluginCameraIntrinsicCalibration::getName() { return "Camera Intrinsic Calibration"; }

QWidget *PluginCameraIntrinsicCalibration::getControlWidget() { return static_cast<QWidget *>(worker->widget); }

ProcessResult PluginCameraIntrinsicCalibration::process(FrameData *data, RenderOptions *options) {
  (void)options;

  Image<raw8> *img_calibration;
  if ((img_calibration = reinterpret_cast<Image<raw8> *>(data->map.get("img_calibration"))) == nullptr) {
    img_calibration = reinterpret_cast<Image<raw8> *>(data->map.insert("img_calibration", new Image<raw8>()));
  }

  ConversionsGreyscale::cvColor2Grey(data->video, img_calibration);

  Chessboard *chessboard;
  if ((chessboard = reinterpret_cast<Chessboard *>(data->map.get("chessboard"))) == nullptr) {
    chessboard = reinterpret_cast<Chessboard *>(data->map.insert("chessboard", new Chessboard()));
  }

  if (data->map.get("chessboard_img_points") == nullptr) {
    data->map.insert("chessboard_img_points", &worker->image_points);
  }

  // cv expects row major order and image stores col major.
  // height and width are swapped intentionally!
  cv::Mat greyscale_mat(img_calibration->getHeight(), img_calibration->getWidth(), CV_8UC1, img_calibration->getData());
  worker->imageSize = greyscale_mat.size();

  if (widget->should_load_images) {
    widget->should_load_images = false;
    emit startLoadImages();
  }

  if (widget->patternDetectionEnabled() || widget->isCapturing()) {
    worker->detectChessboard(greyscale_mat, chessboard);
  }

  if (widget->isCapturing() && chessboard->pattern_was_found) {
    double captureDiff = data->video.getTime() - lastChessboardCaptureFrame;
    if (captureDiff < chessboard_capture_dt->getDouble()) {
      return ProcessingOk;
    }
    lastChessboardCaptureFrame = data->video.getTime();

    bool added = worker->addChessboard(chessboard);

    if (added) {
      worker->image_storage->image_save_mutex.lock();
      worker->image_storage->images_to_save.push(greyscale_mat.clone());
      worker->image_storage->image_save_mutex.unlock();
      emit startSaveImages();
    }
  }

  if (widget->should_calibrate) {
    widget->should_calibrate = false;
    emit startCalibration();
  }

  if (widget->should_clear_data) {
    widget->should_clear_data = false;
    worker->clearData();
  }

  return ProcessingOk;
}

PluginCameraIntrinsicCalibrationWorker::PluginCameraIntrinsicCalibrationWorker(CameraParameters &_camera_params,
                                                                               CameraIntrinsicCalibrationWidget *widget)
    : widget(widget), camera_params(_camera_params) {
  corner_sub_pixel_windows_size = new VarInt("window size", 5, 1);
  corner_sub_pixel_max_iterations = new VarInt("max iterations", 30, 1);
  corner_sub_pixel_epsilon = new VarDouble("epsilon", 0.1, 1e-10);
  corner_diff_sq_threshold = new VarDouble("corner sq_diff threshold", 500);
  reduced_image_width = new VarDouble("reduced image width for chessboard detection", 900.0);
  fixFocalLength = new VarBool("Fix focal length", false);
  fixPrinciplePoint = new VarBool("Fix principle point", false);
  fixTangentialDistortion = new VarBool("Fix tangential distortion", true);
  fixK1 = new VarBool("Fix k1", false);
  fixK2 = new VarBool("Fix k2", false);
  fixK3 = new VarBool("Fix k3", false);
  useIntrinsicGuess = new VarBool("Use intrinsic guess", false);

  image_storage = new ImageStorage(widget);

  thread = new QThread();
  thread->setObjectName("IntrinsicCalibration");
  moveToThread(thread);
  thread->start();
}

PluginCameraIntrinsicCalibrationWorker::~PluginCameraIntrinsicCalibrationWorker() {
  thread->quit();
  thread->deleteLater();

  delete image_storage;
  delete corner_sub_pixel_windows_size;
  delete corner_sub_pixel_epsilon;
  delete corner_sub_pixel_max_iterations;
  delete corner_diff_sq_threshold;
  delete reduced_image_width;
  delete fixFocalLength;
  delete fixPrinciplePoint;
  delete fixTangentialDistortion;
  delete fixK1;
  delete fixK2;
  delete fixK3;
  delete useIntrinsicGuess;
}

void PluginCameraIntrinsicCalibrationWorker::calibrate() {
  calib_mutex.lock();
  widget->calibrating = true;
  widget->updateConfigurationEnabled();

  std::vector<cv::Mat> rvecs;
  std::vector<cv::Mat> tvecs;

  int flags = 0;
  if (fixFocalLength->getBool()) {
    flags |= cv::CALIB_FIX_FOCAL_LENGTH;
  }
  if (fixPrinciplePoint->getBool()) {
    flags |= cv::CALIB_FIX_PRINCIPAL_POINT;
  }
  if (fixTangentialDistortion->getBool()) {
    flags |=cv::CALIB_FIX_TANGENT_DIST;
  }
  if (fixK1->getBool()) {
    flags |=cv::CALIB_FIX_K1;
  }
  if (fixK2->getBool()) {
    flags |=cv::CALIB_FIX_K2;
  }
  if (fixK3->getBool()) {
    flags |=cv::CALIB_FIX_K3;
  }
  if (useIntrinsicGuess->getBool()) {
    flags |= cv::CALIB_USE_INTRINSIC_GUESS;
  }

  try {
    std::cout << "Start calibrating with " << image_points.size() << " samples" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    double rms =
        cv::calibrateCamera(object_points, image_points, imageSize, camera_params.intrinsic_parameters->camera_mat,
                            camera_params.intrinsic_parameters->dist_coeffs, rvecs, tvecs, flags);

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Calibration finished with RMS=" << rms << " in " << elapsed.count() << "s" << std::endl;
    camera_params.intrinsic_parameters->updateConfigValues();
    this->widget->setRms(rms);
  } catch (cv::Exception &e) {
    std::cerr << "calibration failed: " << e.err << std::endl;
    this->widget->setRms(-1);
  }

  widget->calibrating = false;
  widget->updateConfigurationEnabled();
  calib_mutex.unlock();
}

bool PluginCameraIntrinsicCalibrationWorker::addChessboard(const Chessboard *chessboard) {
  calib_mutex.lock();

  // Check if there is a similar sample already
  for (const auto &img_points : this->image_points) {
    double sq_diff_sum = 0;
    for (uint i = 0; i < img_points.size(); i++) {
      double diff = cv::norm(img_points[i] - chessboard->corners[i]);
      sq_diff_sum += diff * diff;
    }
    double sq_diff = sq_diff_sum / (double)img_points.size();
    if (sq_diff < this->corner_diff_sq_threshold->getDouble()) {
      calib_mutex.unlock();
      return false;
    }
  }

  this->image_points.push_back(chessboard->corners);

  std::vector<cv::Point3f> obj;
  for (int y = 0; y < chessboard->pattern_size.height; y++) {
    for (int x = 0; x < chessboard->pattern_size.width; x++) {
      obj.emplace_back(x, y, 0.0f);
    }
  }
  this->object_points.push_back(obj);

  this->widget->setNumDataPoints((int)this->object_points.size());
  calib_mutex.unlock();
  return true;
}

void PluginCameraIntrinsicCalibrationWorker::detectChessboard(const cv::Mat &greyscale_mat,
                                                              Chessboard *chessboard) const {
  chessboard->pattern_size.height = this->camera_params.additional_calibration_information->grid_height->getInt();
  chessboard->pattern_size.width = this->camera_params.additional_calibration_information->grid_width->getInt();
  chessboard->corners.clear();

  cv::Mat greyscale_mat_low_res;
  double scale_factor = min(1.0, reduced_image_width->getDouble() / greyscale_mat.size().width);
  cv::resize(greyscale_mat, greyscale_mat_low_res, cv::Size(), scale_factor, scale_factor);

  std::vector<cv::Point2f> corners_low_res;
  chessboard->pattern_was_found = this->findPattern(greyscale_mat_low_res, chessboard->pattern_size, corners_low_res);

  for (auto &corner : corners_low_res) {
    double x = corner.x / scale_factor;
    double y = corner.y / scale_factor;
    chessboard->corners.push_back(cv::Point((int) x, (int) y));
  }

  if (chessboard->pattern_was_found && this->widget->cornerSubPixCorrectionEnabled()) {
    cv::cornerSubPix(
        greyscale_mat, chessboard->corners,
        cv::Size(corner_sub_pixel_windows_size->getInt(), corner_sub_pixel_windows_size->getInt()), cv::Size(-1, -1),
        cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, corner_sub_pixel_max_iterations->getInt(),
                         corner_sub_pixel_epsilon->getDouble()));
  }
}

bool PluginCameraIntrinsicCalibrationWorker::findPattern(const cv::Mat &image, const cv::Size &pattern_size,
                                                         vector<cv::Point2f> &corners) const {
  using Pattern = CameraIntrinsicCalibrationWidget::Pattern;
  int cb_flags = cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK + cv::CALIB_CB_NORMALIZE_IMAGE;

  switch (widget->getPattern()) {
    case Pattern::CHECKERBOARD:
      return cv::findChessboardCorners(image, pattern_size, corners, cb_flags);
    case Pattern::CIRCLES:
      return cv::findCirclesGrid(image, pattern_size, corners);
    case Pattern::ASYMMETRIC_CIRCLES:
      return cv::findCirclesGrid(image, pattern_size, corners, cv::CALIB_CB_ASYMMETRIC_GRID);
    default:
      return false;
  }
}

void PluginCameraIntrinsicCalibrationWorker::loadImages() {
  std::vector<cv::Mat> images;
  image_storage->readImages(images);

  int n = 0;
  for (cv::Mat &mat : images) {
    Chessboard image_chessboard;
    detectChessboard(mat, &image_chessboard);
    if (image_chessboard.pattern_was_found) {
      bool added = addChessboard(&image_chessboard);
      if (added) {
        std::cout << "Added chessboard" << std::endl;
      } else {
        std::cout << "Filtered chessboard" << std::endl;
      }
    } else {
      std::cout << "No chessboard detected" << std::endl;
    }
    n++;
    widget->setImagesLoaded(n, (int) images.size());
  }
  calibrate();
  widget->imagesLoaded();
}

void PluginCameraIntrinsicCalibrationWorker::clearData() {
  calib_mutex.lock();

  image_points.clear();
  object_points.clear();

  widget->setNumDataPoints((int) object_points.size());
  widget->setRms(0.0);

  calib_mutex.unlock();
}

ImageStorage::ImageStorage(CameraIntrinsicCalibrationWidget *widget) : widget(widget) {
  image_dir = new VarString("pattern image dir", "test-data/intrinsic_calibration");

  thread = new QThread();
  thread->setObjectName("IntrinsicCalibrationImageStorage");
  moveToThread(thread);
  thread->start();
}

ImageStorage::~ImageStorage() {
  thread->quit();
  thread->deleteLater();
  delete image_dir;
}

void ImageStorage::saveImages() {
  image_save_mutex.lock();
  if (images_to_save.empty()) {
    image_save_mutex.unlock();
  } else {
    cv::Mat image = images_to_save.front();
    images_to_save.pop();
    image_save_mutex.unlock();
    saveImage(image);
    saveImages();
  }
}

void ImageStorage::saveImage(cv::Mat &image) const {
  long t_now = (long)(GetTimeSec() * 1e9);
  QString num = QString::number(t_now);
  QString filename = QString(image_dir->getString().c_str()) + "/" + num + ".png";
  cv::imwrite(filename.toStdString(), image);
}

static bool hasEnding (std::string const &fullString, std::string const &ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

void ImageStorage::readImages(std::vector<cv::Mat> &images) const {
  DIR *dp;
  if ((dp = opendir(image_dir->getString().c_str())) == nullptr) {
    std::cerr << "Failed to open directory: " << image_dir->getString() << std::endl;
    return;
  }
  struct dirent *dirp;
  std::list<std::string> imgs_to_load(0);
  while ((dirp = readdir(dp))) {
    std::string file_name(dirp->d_name);
    if (file_name[0] != '.' && hasEnding(file_name, ".png")) {  // not a hidden file or one of '..' or '.'
      imgs_to_load.push_back(image_dir->getString() + "/" + file_name);
    }
  }
  closedir(dp);

  imgs_to_load.sort();
  for (const auto &currentImage : imgs_to_load) {
    std::cout << "Loading " << currentImage << std::endl;
    cv::Mat srcImg = imread(currentImage, cv::IMREAD_GRAYSCALE);
    images.push_back(srcImg);
    widget->setImagesLoaded(0, (int) images.size());
  }
}
