#pragma once

#include <QPushButton>
#include <QWidget>

/*!
  \file    maskwidget.h
  \brief   A GUI for setting up a mask on the image
  \author  Rui Silva, (C) 2017
*/
class MaskWidget : public QWidget {
  Q_OBJECT
public:
  MaskWidget();
  ~MaskWidget() override = default;

  QPushButton *clear_mask_button;
};
