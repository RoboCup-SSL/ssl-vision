#include "maskwidget.h"
#include <QListWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>

MaskWidget::MaskWidget() {

  auto mask_label = new QLabel(tr(
      "Draw a mask around the field to exclude the area outside of the mask. "
      "You can add points to the image with a left click."
      "The points form a polygonal shape "
      "that builds up the mask.\n"
      "Hold Shift to remove a point. "
      "Make sure to enable the visualization of the mask in the config tree."));
  mask_label->setWordWrap(true);

  clear_mask_button = new QPushButton(tr("Clear mask"));
  clear_mask_button->setCheckable(true);

  auto mask_group_box = new QGroupBox(tr("Vision Mask"));
  auto maskLayout = new QVBoxLayout;
  mask_group_box->setLayout(maskLayout);
  maskLayout->addWidget(mask_label);
  maskLayout->addWidget(clear_mask_button);
  maskLayout->addStretch();

  auto vbox = new QVBoxLayout;
  vbox->addWidget(mask_group_box);
  setLayout(vbox);
}
