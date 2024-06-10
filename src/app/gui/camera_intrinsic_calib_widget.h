#ifndef CAMERA_INTRINSIC_CALIB_WIDGET_H
#define CAMERA_INTRINSIC_CALIB_WIDGET_H

#include <camera_calibration.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

class CameraIntrinsicCalibrationWidget : public QWidget {
  Q_OBJECT
 public:
  enum class Pattern : int { CHECKERBOARD = 0, CIRCLES, ASYMMETRIC_CIRCLES };

 public:
  explicit CameraIntrinsicCalibrationWidget(CameraParameters &camera_params);
  ~CameraIntrinsicCalibrationWidget() override = default;

  CameraParameters &camera_params;

 protected:
  QComboBox *pattern_selector;
  QSpinBox *grid_width;
  QSpinBox *grid_height;
  QCheckBox *detect_pattern_checkbox;
  QCheckBox *corner_subpixel_correction_checkbox;
  QLabel *num_data_points_label;
  QLabel *rms_label;
  QLabel *images_loaded_label;
  QPushButton *clear_data_button;
  QPushButton *capture_button;
  QPushButton *calibrate_button;
  QPushButton *load_images_button;
  QPushButton *reset_model_button;

 public:
  bool patternDetectionEnabled() const { return detect_pattern_checkbox->isChecked(); }
  bool cornerSubPixCorrectionEnabled() const { return corner_subpixel_correction_checkbox->isChecked(); }
  bool isCapturing() const { return capture_button->isChecked(); }
  bool isLoadingFiles() const { return should_load_images; }
  bool isConfigurationEnabled() const { return !isCapturing() && !isLoadingFiles() && !calibrating; }
  void setNumDataPoints(int n);
  Pattern getPattern() const { return static_cast<Pattern>(pattern_selector->currentIndex()); }
  void setImagesLoaded(int n, int total);
  void imagesLoaded();

 public slots:
  void clearDataClicked();
  void calibrateClicked();
  void updateConfigurationEnabled();
  void loadImagesClicked();
  void grid_height_changed(int) const;
  void grid_height_vartype_changed(VarType* varType);
  void grid_width_changed(int) const;
  void grid_width_vartype_changed(VarType* varType);
  void resetModelClicked() const;

 public:
  void setRms(double rms);

 public:
  bool should_clear_data = false;
  bool should_load_images = false;
  bool should_calibrate = false;
  bool calibrating = false;
};

#endif /* CAMERA_INTRINSIC_CALIB_WIDGET_H */
