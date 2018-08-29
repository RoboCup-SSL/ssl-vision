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

  QLabel* globalCameraIdLabel = new QLabel("Global camera id ");
  globalCameraId = new QLineEdit(QString::number(camera_parameters.additional_calibration_information->camera_index->getInt()));
  globalCameraId->setValidator(new QIntValidator(0, 7, this));
  globalCameraId->setMaximumWidth(30);
  QPushButton* updateControlPointsButton = new QPushButton(tr("Update control points"));
  connect(updateControlPointsButton, SIGNAL(clicked()), SLOT(is_clicked_update_control_points()));
  QPushButton* initialCalibrationButton = new QPushButton(tr("Do initial calibration"));
  connect(initialCalibrationButton, SIGNAL(clicked()), SLOT(is_clicked_initial()));
  QPushButton* fullCalibrationButton = new QPushButton(tr("Do full calibration"));
  connect(fullCalibrationButton, SIGNAL(clicked()), SLOT(is_clicked_full()));
  QPushButton* additionalPointsButton = new QPushButton(tr("Detect additional calibration points"));
  connect(additionalPointsButton, SIGNAL(clicked()), SLOT(edges_is_clicked()));
  QPushButton* resetButton = new QPushButton(tr("Reset"));
  connect(resetButton, SIGNAL(clicked()), SLOT(is_clicked_reset()));
  
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
  QGroupBox* globalCameraSelectBox = new QGroupBox();
  globalCameraSelectBox->setLayout(hbox);

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(globalCameraSelectBox);
  vbox->addWidget(initialCalibrationButton);
  vbox->addWidget(additionalPointsButton);
  vbox->addWidget(fullCalibrationButton);
  vbox->addWidget(resetButton);
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
  camera_parameters.do_calibration(CameraParameters::FOUR_POINT_INITIAL);
  set_slider_from_vars();
}

void CameraCalibrationWidget::is_clicked_full()
{
  camera_parameters.do_calibration(CameraParameters::FULL_ESTIMATION);
  set_slider_from_vars();
}

void CameraCalibrationWidget::is_clicked_reset()
{
  camera_parameters.reset();
  set_slider_from_vars();
}

void CameraCalibrationWidget::edges_is_clicked()
{
  detectEdges = true;
}

void CameraCalibrationWidget::set_slider_from_vars()
{
  cameraHeightSlider->setValue((int)camera_parameters.tz->getDouble());
  distortionSlider->setValue((int)(camera_parameters.distortion->getDouble()*100));
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
