#pragma once

#include <QThread>
#include <VarTypes.h>
#include <camera_calibration.h>
#include <camera_intrinsic_calib_widget.h>
#include <camera_parameters.h>
#include <framedata.h>
#include <image.h>
#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <visionplugin.h>

class Chessboard {
public:
  std::vector<cv::Point2f> corners;
  cv::Size pattern_size;
  bool pattern_was_found;
};

class ImageStorage : public QObject {
  Q_OBJECT
public:
  explicit ImageStorage(CameraIntrinsicCalibrationWidget *widget);
  ~ImageStorage() override;
  QThread *thread;
  CameraIntrinsicCalibrationWidget *widget;

  std::mutex image_save_mutex;
  std::queue<cv::Mat> images_to_save;

  VarString *image_dir;

  void saveImage(cv::Mat& image) const;
  void readImages(std::vector<cv::Mat> &images) const;

public slots:
  void saveImages();
};

class PluginCameraIntrinsicCalibrationWorker : public QObject {
  Q_OBJECT
public:
  PluginCameraIntrinsicCalibrationWorker(
      CameraParameters &_camera_params,
      CameraIntrinsicCalibrationWidget *widget);
  ~PluginCameraIntrinsicCalibrationWorker() override;
  QThread *thread;
  std::mutex calib_mutex;

  CameraIntrinsicCalibrationWidget *widget;
  ImageStorage *image_storage;

  std::vector<std::vector<cv::Point3f>> object_points;
  std::vector<std::vector<cv::Point2f>> image_points;
  cv::Size imageSize;

  VarInt *corner_sub_pixel_windows_size;
  VarInt *corner_sub_pixel_max_iterations;
  VarDouble *corner_sub_pixel_epsilon;
  VarDouble *corner_diff_sq_threshold;
  VarDouble *reduced_image_width;
  VarBool *fixFocalLength;
  VarBool *fixPrinciplePoint;
  VarBool *fixTangentialDistortion;
  VarBool *fixK1;
  VarBool *fixK2;
  VarBool *fixK3;
  VarBool *useIntrinsicGuess;

  void detectChessboard(const cv::Mat &greyscale_mat,
                        Chessboard *chessboard) const;
  bool findPattern(const cv::Mat &image, const cv::Size &pattern_size,
                   vector<cv::Point2f> &corners) const;

  bool addChessboard(const Chessboard *chessboard);

  void clearData();

public slots:
  void loadImages();
  void calibrate();

private:
  CameraParameters camera_params;
};

class PluginCameraIntrinsicCalibration : public VisionPlugin {
  Q_OBJECT
protected:
  std::unique_ptr<VarList> settings;

public:
  PluginCameraIntrinsicCalibration(FrameBuffer *_buffer,
                                   CameraParameters &camera_params);
  ~PluginCameraIntrinsicCalibration() override;

  QWidget *getControlWidget() override;

  ProcessResult process(FrameData *data, RenderOptions *options) override;
  VarList *getSettings() override;
  std::string getName() override;

signals:
  void startLoadImages();
  void startCalibration();
  void startSaveImages();

private:
  PluginCameraIntrinsicCalibrationWorker *worker;

  CameraIntrinsicCalibrationWidget *widget;
  CameraParameters camera_params;

  VarDouble *chessboard_capture_dt;

  double lastChessboardCaptureFrame = 0.0;
};
