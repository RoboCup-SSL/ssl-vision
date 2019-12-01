#include "maskwidget.h"
#include <QListWidget>
#include <QGroupBox>
#include <QVBoxLayout>

MaskWidget::MaskWidget() {
  QGroupBox* maskBox = new QGroupBox(tr("Mask"));
  QVBoxLayout *vbox = new QVBoxLayout;
  maskBox->setLayout(vbox);

  QVBoxLayout *vbox2 = new QVBoxLayout;
  vbox2->addWidget(maskBox);
  this->setLayout(vbox2);
}

MaskWidget::~MaskWidget()
{
  // Destroy GUI here
}
