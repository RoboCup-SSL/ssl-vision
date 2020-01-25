#include "maskwidget.h"
#include <QListWidget>
#include <QGroupBox>
#include <QVBoxLayout>

MaskWidget::MaskWidget() {
  auto maskBox = new QGroupBox(tr("Mask"));
  auto vbox = new QVBoxLayout;
  maskBox->setLayout(vbox);

  auto vbox2 = new QVBoxLayout;
  vbox2->addWidget(maskBox);
  this->setLayout(vbox2);
}
