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
  \file    automatedcolorcalibwidget.cpp
  \brief   C++ Implementation: AutomatedColorCalibWidget
  \author  Mark Geiger, (C) 2017
*/
//========================================================================


#include "automatedcolorcalibwidget.h"
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>

AutomatedColorCalibWidget::AutomatedColorCalibWidget(const LUT3D *lut) {

  QGroupBox *calibrationStepsBox = new QGroupBox(tr("Calibration Steps"));

  QPushButton *resetLUTButton = new QPushButton(tr("Clear LUT"));
  connect(resetLUTButton, SIGNAL(clicked()), SLOT(resetLut()));

  QPushButton *resetCalibrationButton = new QPushButton(tr("Remove all samples"));
  connect(resetCalibrationButton, SIGNAL(clicked()), SLOT(reset()));

  QPushButton *initialCalibrationButton = new QPushButton(tr("Initialize LUT"));
  connect(initialCalibrationButton, SIGNAL(clicked()), SLOT(initialize()));

  QPushButton *updateCalibrationButton = new QPushButton(tr("Update LUT"));
  connect(updateCalibrationButton, SIGNAL(clicked()), SLOT(update()));

  list = new QListWidget(this);
  list->setGeometry(QRect(50, 40, 120, 200));
  updateList(lut);
  connect(list, SIGNAL(currentRowChanged(int)), this, SLOT(selectChannel(int)));
  list->setFixedWidth(list->sizeHintForColumn(0) + 5);
  list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  list->setFocusPolicy(Qt::NoFocus);
  setFocusPolicy(Qt::StrongFocus);

  auto *samplesLayout = new QHBoxLayout;
  samplesLayout->addWidget(list);
  samplesLayout->addWidget(new QLabel(
          "Select color and collect samples in image\nYou can find the collected samples in the config tree on the left"));
  auto* samplesBox = new QGroupBox();
  samplesBox->setLayout(samplesLayout);

  auto *vBox = new QVBoxLayout;
  vBox->addWidget(resetLUTButton);
  vBox->addWidget(resetCalibrationButton);
  vBox->addWidget(samplesBox);
  vBox->addWidget(initialCalibrationButton);
  vBox->addWidget(updateCalibrationButton);

  auto *vBoxWrapper = new QVBoxLayout;
  vBoxWrapper->addLayout(vBox, 1);
  calibrationStepsBox->setLayout(vBoxWrapper);

  status_label = new QLabel("/");

  QGroupBox *statusBox = new QGroupBox(tr("Status"));
  auto *statusGrid = new QGridLayout;
  statusGrid->addWidget(new QLabel("Status: "), 0, 0, 1, 1);
  statusGrid->addWidget(status_label, 0, 1, 1, 2);
  statusGrid->setColumnStretch(1, 1);
  statusBox->setLayout(statusGrid);

  // Overall layout:
  auto *vbox2 = new QVBoxLayout;
  vbox2->addWidget(calibrationStepsBox);
  vbox2->addStrut(5);
  vbox2->addStretch(1);
  vbox2->addWidget(statusBox);
  this->setLayout(vbox2);
}

void AutomatedColorCalibWidget::selectChannel(int c) {
  if (c != -1) {
    currentChannel = c;
  }
}

void AutomatedColorCalibWidget::updateList(const LUT3D *lut) {
  QListWidgetItem *item;
  for (const auto &channel : lut->channels) {
    item = new QListWidgetItem(QString::fromStdString(channel.label));
    QPixmap p(16, 16);
    p.fill(QColor(channel.draw_color.r, channel.draw_color.g, channel.draw_color.b));
    QPainter painter(&p);
    painter.setPen(QPen(QColor(0, 0, 0), 1));
    painter.drawRect(0, 0, 15, 15);

    item->setIcon(p);
    list->addItem(item);
  }
}

void AutomatedColorCalibWidget::focusInEvent(QFocusEvent *event) {
  (void) event;
}

void AutomatedColorCalibWidget::reset() {
  pending_reset = true;
}

void AutomatedColorCalibWidget::resetLut() {
  pending_reset_lut = true;
}

void AutomatedColorCalibWidget::initialize() {
  pending_reset_lut = true;
  pending_update = true;
}

void AutomatedColorCalibWidget::update() {
  pending_update = true;
}

void AutomatedColorCalibWidget::set_status(const QString &status) {
  status_label->setText(status);
}
