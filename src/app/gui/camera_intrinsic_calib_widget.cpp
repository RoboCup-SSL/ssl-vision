#include "camera_intrinsic_calib_widget.h"

#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

CameraIntrinsicCalibrationWidget::CameraIntrinsicCalibrationWidget(CameraParameters& camera_params)
    : camera_params{camera_params} {
  auto calibration_steps_layout = new QVBoxLayout;

  // pattern configuration
  {
    auto pattern_config_layout = new QVBoxLayout;

    // pattern select dropdown
    {
      auto pattern_selector_label = new QLabel(tr("Pattern type:"));

      pattern_selector = new QComboBox();
      pattern_selector->addItems({"Checkerboard", "Circles", "Asymmetric Circles"});

      auto hbox = new QHBoxLayout;
      hbox->addWidget(pattern_selector_label);
      hbox->addWidget(pattern_selector);

      pattern_config_layout->addLayout(hbox);
    }

    // pattern size config
    {
      auto grid_dimensions_label = new QLabel(tr("Grid Dimensions (width x height):"));

      grid_width = new QSpinBox();
      grid_width->setMinimum(2);
      grid_width->setValue(camera_params.additional_calibration_information->grid_width->getInt());
      connect(grid_width, SIGNAL(valueChanged(int)), this, SLOT(grid_width_changed(int)));
      connect(camera_params.additional_calibration_information->grid_width, SIGNAL(hasChanged(VarType*)), this,
              SLOT(grid_width_vartype_changed(VarType*)));

      auto grid_dim_separator_label = new QLabel(tr("x"));

      grid_height = new QSpinBox();
      grid_height->setMinimum(2);
      grid_height->setValue(camera_params.additional_calibration_information->grid_height->getInt());
      connect(grid_height, SIGNAL(valueChanged(int)), this, SLOT(grid_height_changed(int)));
      connect(camera_params.additional_calibration_information->grid_height, SIGNAL(hasChanged(VarType*)), this,
              SLOT(grid_height_vartype_changed(VarType*)));

      auto hbox = new QHBoxLayout;
      hbox->addWidget(grid_dimensions_label);
      hbox->addStretch();
      hbox->addWidget(grid_width);
      hbox->addWidget(grid_dim_separator_label);
      hbox->addWidget(grid_height);

      pattern_config_layout->addLayout(hbox);
    }

    auto pattern_config_groupbox = new QGroupBox(tr("Pattern Configuration"));
    pattern_config_groupbox->setLayout(pattern_config_layout);

    calibration_steps_layout->addWidget(pattern_config_groupbox);
  }

  // calibration instructions
  {
    auto calibration_instructions_layout = new QVBoxLayout;

    calibration_instructions_layout->addWidget(
        new QLabel(tr("Enable pattern detection here and enable display in the "
                      "VisualizationPlugin.\n"
                      "Verify that your pattern is detected properly.\nIf not "
                      "detected double check the grid size and pattern type "
                      "and verify that greyscale image has good contrast.")));

    // detect pattern checkbox
    {
      auto label = new QLabel(tr("Detect Pattern"));

      detect_pattern_checkbox = new QCheckBox();

      auto hbox = new QHBoxLayout;
      hbox->addWidget(label);
      hbox->addWidget(detect_pattern_checkbox);

      calibration_instructions_layout->addLayout(hbox);
    }

    // do corner subpixel correction checkbox
    {
      auto label = new QLabel(tr("Do Corner Subpixel Correction:"));

      corner_subpixel_correction_checkbox = new QCheckBox();
      corner_subpixel_correction_checkbox->setChecked(true);

      auto hbox = new QHBoxLayout;
      hbox->addWidget(label);
      hbox->addWidget(corner_subpixel_correction_checkbox);

      calibration_instructions_layout->addLayout(hbox);

      calibration_instructions_layout->addWidget(
          new QLabel(tr("When you can see a chessboard in the image, you can "
                        "start capturing data.\n"
                        "After each new sample, "
                        "a calibration will be done and the "
                        "calibration error will be shown as RMS.\n"
                        "Make sure to capture enough images from different "
                        "poses (like ~30).")));

      capture_button = new QPushButton(tr("Capture"));
      capture_button->setCheckable(true);
      connect(capture_button, SIGNAL(clicked()), this, SLOT(updateConfigurationEnabled()));
      calibration_instructions_layout->addWidget(capture_button);

      calibrate_button = new QPushButton(tr("Calibrate"));
      connect(calibrate_button, SIGNAL(clicked()), this, SLOT(calibrateClicked()));
      calibration_instructions_layout->addWidget(calibrate_button);

      calibration_instructions_layout->addWidget(
          new QLabel(tr("Images where a chessboard was detected "
                        "are saved in 'test-data/intrinsic_calibration'.\n"
                        "You can load all images again to redo or tune "
                        "the calibration.")));

      load_images_button = new QPushButton(tr("Load saved images"));
      connect(load_images_button, SIGNAL(clicked()), this, SLOT(loadImagesClicked()));
      calibration_instructions_layout->addWidget(load_images_button);

      reset_model_button = new QPushButton(tr("Reset model"));
      connect(reset_model_button, SIGNAL(clicked()), this, SLOT(resetModelClicked()));
      calibration_instructions_layout->addWidget(reset_model_button);
    }

    // images loaded
    {
      auto hbox = new QHBoxLayout;
      hbox->addWidget(new QLabel(tr("Images loaded: ")));

      images_loaded_label = new QLabel(tr("0 / 0"));
      hbox->addWidget(images_loaded_label);

      calibration_instructions_layout->addLayout(hbox);
    }

    auto calibration_instructions_groupbox = new QGroupBox(tr("Calibration Instructions"));
    calibration_instructions_groupbox->setLayout(calibration_instructions_layout);

    calibration_steps_layout->addWidget(calibration_instructions_groupbox);
  }

  // capture control buttons
  {
    auto capture_control_layout = new QVBoxLayout;
    auto capture_control_groupbox = new QGroupBox(tr("Calibration Data"));
    capture_control_groupbox->setLayout(capture_control_layout);

    // captured data info
    {
      auto hbox = new QHBoxLayout;
      hbox->addWidget(new QLabel(tr("Number of data points: ")));

      num_data_points_label = new QLabel(tr("0"));
      hbox->addWidget(num_data_points_label);

      capture_control_layout->addLayout(hbox);
    }

    // calibration RMS error
    {
      auto hbox = new QHBoxLayout;
      hbox->addWidget(new QLabel(tr("Calibration RMS: ")));

      rms_label = new QLabel(tr("-"));
      hbox->addWidget(rms_label);

      capture_control_layout->addLayout(hbox);
    }

    capture_control_layout->addWidget(
        new QLabel(tr("The calibration result can be found under \n"
                      "Camera Calibrator -> Camera Parameters -> Intrinsic Parameters")));

    capture_control_layout->addSpacing(50);

    // control buttons
    {
      clear_data_button = new QPushButton(tr("Clear Data"));
      connect(clear_data_button, SIGNAL(clicked()), this, SLOT(clearDataClicked()));

      auto hbox = new QHBoxLayout;
      hbox->addWidget(clear_data_button);

      capture_control_layout->addLayout(hbox);
    }

    calibration_steps_layout->addWidget(capture_control_groupbox);
  }

  // push widgets to top
  calibration_steps_layout->addStretch();

  this->setLayout(calibration_steps_layout);
}

void CameraIntrinsicCalibrationWidget::setNumDataPoints(int n) { num_data_points_label->setText(QString("%1").arg(n)); }

void CameraIntrinsicCalibrationWidget::clearDataClicked() {
  setImagesLoaded(0, 0);
  should_clear_data = true;
}

void CameraIntrinsicCalibrationWidget::calibrateClicked() {
  should_calibrate = true;
  updateConfigurationEnabled();
}

void CameraIntrinsicCalibrationWidget::updateConfigurationEnabled() {
  pattern_selector->setEnabled(isConfigurationEnabled());
  grid_width->setEnabled(isConfigurationEnabled());
  grid_height->setEnabled(isConfigurationEnabled());
  clear_data_button->setEnabled(isConfigurationEnabled());
  detect_pattern_checkbox->setEnabled(isConfigurationEnabled());
  corner_subpixel_correction_checkbox->setEnabled(isConfigurationEnabled());
  load_images_button->setEnabled(isConfigurationEnabled());
  calibrate_button->setEnabled(isConfigurationEnabled());
  reset_model_button->setEnabled(isConfigurationEnabled());
}

void CameraIntrinsicCalibrationWidget::loadImagesClicked() {
  setImagesLoaded(0, 0);
  should_load_images = true;
  updateConfigurationEnabled();
}

void CameraIntrinsicCalibrationWidget::resetModelClicked() const { camera_params.intrinsic_parameters->reset(); }

void CameraIntrinsicCalibrationWidget::setRms(double rms) { rms_label->setText(QString("%1").arg(rms)); }

void CameraIntrinsicCalibrationWidget::grid_height_changed(int height) const {
  camera_params.additional_calibration_information->grid_height->setInt(height);
}

void CameraIntrinsicCalibrationWidget::grid_height_vartype_changed(VarType* varType) {
  grid_height->setValue(((VarInt*)varType)->getInt());
}

void CameraIntrinsicCalibrationWidget::grid_width_changed(int width) const {
  camera_params.additional_calibration_information->grid_width->setInt(width);
}

void CameraIntrinsicCalibrationWidget::grid_width_vartype_changed(VarType* varType) {
  grid_width->setValue(((VarInt*)varType)->getInt());
}

void CameraIntrinsicCalibrationWidget::setImagesLoaded(int n, int total) {
  images_loaded_label->setText(QString("%1 / %2").arg(n).arg(total));
}

void CameraIntrinsicCalibrationWidget::imagesLoaded() { updateConfigurationEnabled(); }
