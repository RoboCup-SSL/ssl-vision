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
/*!
  \file    camcalibwidget.cpp
  \brief   C++ Implementation: CameraCalibrationWidget
  \author  OB, (C) 2008
*/
//========================================================================


#include "cameracalibwidget.h"
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <iostream>

CameraCalibrationWidget::CameraCalibrationWidget(CameraParameters &_cp) : camera_parameters(_cp), detectEdges(false)
{
  // The calibration points and the fit button:
  QGroupBox* calibrationStepsBox = new QGroupBox(tr("Calibration Steps"));

  auto globalCameraIdLabel = new QLabel("Update control points based on the "
                                        "<a "
                                        "href=\"https://github.com/RoboCup-SSL/"
                                        "ssl-vision/wiki/camera-calibration\">"
                                        "global camera id</a>: ");
  globalCameraIdLabel->setTextFormat(Qt::RichText);
  globalCameraIdLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  globalCameraIdLabel->setOpenExternalLinks(true);

  globalCameraId = new QLineEdit(QString::number(camera_parameters.additional_calibration_information->global_camera_id->getInt()));
  globalCameraId->setValidator(new QIntValidator(0, 7, this));
  globalCameraId->setMaximumWidth(30);
  connect(globalCameraId, SIGNAL(textChanged(QString)),
      this, SLOT(global_camera_id_changed()));
  connect(camera_parameters.additional_calibration_information->global_camera_id, SIGNAL(hasChanged(VarType*)),
      this, SLOT(global_camera_id_vartype_changed()));

  openCvCalibrationCheckBox = new QCheckBox("Use OpenCV calibration with separate intrinsic and extrinsic model");
  openCvCalibrationCheckBox->setChecked(camera_parameters.use_opencv_model->getBool());
  connect(openCvCalibrationCheckBox, SIGNAL(stateChanged(int)), this, SLOT(opencvCalibrationTypeChanged(int)));
  connect(camera_parameters.use_opencv_model, SIGNAL(hasChanged(VarType *)), this, SLOT(opencvCalibrationTypeChangedVarType()));

  QPushButton* updateControlPointsButton = new QPushButton(tr("Update control points"));
  connect(updateControlPointsButton, SIGNAL(clicked()), SLOT(is_clicked_update_control_points()));
  QPushButton* initialCalibrationButton = new QPushButton(tr("Do initial calibration"));
  connect(initialCalibrationButton, SIGNAL(clicked()), SLOT(is_clicked_initial()));
  fullCalibrationButton = new QPushButton(tr("Do full calibration"));
  connect(fullCalibrationButton, SIGNAL(clicked()), SLOT(is_clicked_full()));
  QPushButton* additionalPointsButton = new QPushButton(tr("Detect additional calibration points"));
  connect(additionalPointsButton, SIGNAL(clicked()), SLOT(edges_is_clicked()));
  QPushButton* resetButton = new QPushButton(tr("Reset"));
  connect(resetButton, SIGNAL(clicked()), SLOT(is_clicked_reset()));

  auto errorDescriptionLabel = new QLabel("Root Mean Squared Error (RMSE): ");
  errorValueLabel = new QLabel("");

  QGroupBox* calibrationParametersBox = new QGroupBox(tr("Calibration Parameters"));
  // The slider for the width of the line search corridor:
  QLabel* widthLabel = new QLabel("Line Search Corridor Width (in mm) ");
  lineSearchCorridorWidthSlider = new QSlider(Qt::Horizontal);
  lineSearchCorridorWidthSlider->setMinimum(50);
  lineSearchCorridorWidthSlider->setMaximum(800);
  lineSearchCorridorWidthSlider->setValue((int)(camera_parameters.additional_calibration_information->line_search_corridor_width->getDouble()));
  lineSearchCorridorWidthLabelRight = new QLabel();
  lineSearchCorridorWidthLabelRight->setNum(200);
  connect(lineSearchCorridorWidthSlider, SIGNAL(valueChanged(int)), this, SLOT(line_search_slider_changed(int)));

  QGroupBox* cameraParametersBox = new QGroupBox(tr("Initial Camera Parameters"));
  // The slider for height control:
  QLabel* heightLabel = new QLabel("Camera Height (in mm) ");
  cameraHeightSlider = new QSlider(Qt::Horizontal);
  cameraHeightSlider->setMinimum((int)camera_parameters.tz->getMin());
  cameraHeightSlider->setMaximum((int)camera_parameters.tz->getMax());
  cameraHeightSlider->setValue((int)camera_parameters.tz->getDouble());
  cameraHeightLabelRight = new QLabel();
  cameraHeightLabelRight->setNum((int)camera_parameters.tz->getDouble());
  connect(cameraHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(cameraheight_slider_changed(int)));
  // Distortion slider
  QLabel* distortionLabel = new QLabel("Distortion ");
  distortionSlider = new QSlider(Qt::Horizontal);
  distortionSlider->setMinimum((int)(camera_parameters.distortion->getMin()*100));
  distortionSlider->setMaximum((int)(camera_parameters.distortion->getMax()*100));
  distortionSlider->setValue((int)(camera_parameters.distortion->getDouble()*100));
  distortionLabelRight = new QLabel();
  distortionLabelRight->setNum(1./100. * (double)(distortionSlider->value()));
  connect(distortionSlider, SIGNAL(valueChanged(int)), this, SLOT(distortion_slider_changed(int)));

  // Layout for calibration control:
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(globalCameraIdLabel);
  hbox->addWidget(globalCameraId);
  hbox->addWidget(updateControlPointsButton);
  hbox->addStretch(1);

  auto errorLayout = new QHBoxLayout;
  errorLayout->addWidget(errorDescriptionLabel);
  errorLayout->addWidget(errorValueLabel);
  errorLayout->addStretch(1);

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addLayout(hbox);
  vbox->addWidget(openCvCalibrationCheckBox);
  vbox->addWidget(initialCalibrationButton);
  vbox->addWidget(additionalPointsButton);
  vbox->addWidget(fullCalibrationButton);
  vbox->addWidget(resetButton);
  vbox->addLayout(errorLayout);
  vbox->addStretch(1);
  calibrationStepsBox->setLayout(vbox);
  // Layout for calibration parameters
  QGridLayout* gridCalibration = new QGridLayout;
  gridCalibration->addWidget(widthLabel,0,0);
  gridCalibration->addWidget(lineSearchCorridorWidthSlider,0,1);
  gridCalibration->addWidget(lineSearchCorridorWidthLabelRight,0,2);
  calibrationParametersBox->setLayout(gridCalibration);
  // Layout for camera parameters
  QGridLayout* gridCamera = new QGridLayout;
  gridCamera->addWidget(heightLabel,0,0);
  gridCamera->addWidget(cameraHeightSlider,0,1);
  gridCamera->addWidget(cameraHeightLabelRight,0,2);
  gridCamera->addWidget(distortionLabel,1,0);
  gridCamera->addWidget(distortionSlider,1,1);
  gridCamera->addWidget(distortionLabelRight,1,2);
  cameraParametersBox->setLayout(gridCamera);

  // Overall layout:
  QVBoxLayout *vbox2 = new QVBoxLayout;
  vbox2->addWidget(calibrationStepsBox);
  vbox2->addWidget(calibrationParametersBox);
  vbox2->addWidget(cameraParametersBox);
  this->setLayout(vbox2);
}

CameraCalibrationWidget::~CameraCalibrationWidget()
{
  // Destroy GUI here
}

void CameraCalibrationWidget::focusInEvent ( QFocusEvent * event ) {
  (void)event;
  //forward the focus to the actual widget that we contain
  // gllut->setFocus(Qt::OtherFocusReason); left as an example for later use
}

void CameraCalibrationWidget::is_clicked_update_control_points()
{
  int cameraId = globalCameraId->text().toInt();
  camera_parameters.additional_calibration_information->camera_index->setInt(cameraId);
  camera_parameters.additional_calibration_information->updateControlPoints();
}

void CameraCalibrationWidget::is_clicked_initial()
{
  double rmse = camera_parameters.do_calibration(CameraParameters::FOUR_POINT_INITIAL);
  errorValueLabel->setText(QString::number(rmse));
  set_slider_from_vars();
}

void CameraCalibrationWidget::is_clicked_full()
{
  double rmse = camera_parameters.do_calibration(CameraParameters::FULL_ESTIMATION);
  errorValueLabel->setText(QString::number(rmse));
  set_slider_from_vars();
}

void CameraCalibrationWidget::is_clicked_reset()
{
  camera_parameters.calibrationSegments.clear();
  if(camera_parameters.use_opencv_model->getBool()) {
    camera_parameters.extrinsic_parameters->reset();
  } else {
    camera_parameters.reset();
  }
  set_slider_from_vars();
}

void CameraCalibrationWidget::edges_is_clicked()
{
  detectEdges = true;
}

void CameraCalibrationWidget::set_slider_from_vars()
{
  if(!camera_parameters.use_opencv_model->getBool()) {
    cameraHeightSlider->setValue((int)camera_parameters.tz->getDouble());
    distortionSlider->setValue(
        (int)(camera_parameters.distortion->getDouble() * 100));
  }
  lineSearchCorridorWidthSlider->setValue((int)(camera_parameters.additional_calibration_information->line_search_corridor_width->getDouble()));
}

void CameraCalibrationWidget::cameraheight_slider_changed(int val)
{
  cameraHeightLabelRight->setNum(val);
  camera_parameters.tz->setDouble(val);
}

void CameraCalibrationWidget::distortion_slider_changed(int val)
{
  double doubleVal = 1/100. * (double) val;
  distortionLabelRight->setNum(doubleVal);
  camera_parameters.distortion->setDouble(doubleVal);
}

void CameraCalibrationWidget::line_search_slider_changed(int val)
{
  camera_parameters.additional_calibration_information->line_search_corridor_width->setDouble(val);
  lineSearchCorridorWidthLabelRight->setNum(val);
}

void CameraCalibrationWidget::global_camera_id_changed() {
  camera_parameters.additional_calibration_information->global_camera_id->setInt(globalCameraId->text().toInt());
}

void CameraCalibrationWidget::global_camera_id_vartype_changed() {
  globalCameraId->setText(QString::number(camera_parameters.additional_calibration_information->global_camera_id->getInt()));
}

void CameraCalibrationWidget::opencvCalibrationTypeChanged(int state) {
  camera_parameters.use_opencv_model->setBool(state == Qt::Checked);
  setEnabledBasedOnModel();
}

void CameraCalibrationWidget::opencvCalibrationTypeChangedVarType() {
  openCvCalibrationCheckBox->setChecked(camera_parameters.use_opencv_model->getBool());
  setEnabledBasedOnModel();
}

void CameraCalibrationWidget::setEnabledBasedOnModel() {
  bool opencv_model = camera_parameters.use_opencv_model->getBool();
  cameraHeightSlider->setEnabled(!opencv_model);
  distortionSlider->setEnabled(!opencv_model);
}
