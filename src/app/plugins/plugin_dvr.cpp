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
  \file    plugin_dvr.cpp
  \brief   C++ Implementation: plugin_dvr
  \author  Stefan Zickler, 2009
*/
//========================================================================
#include "plugin_dvr.h"

PluginDVRWidget::PluginDVRWidget(QWidget * parent, Qt::WindowFlags f) : QWidget(parent,f) {
  layout_main=new QVBoxLayout();

  //group_mode = new QButtonGroup();
  /*group_mode->setExclusive(true);
  group_mode->addButton();
  group_mode->addButton();
  group_mode->addButton();*/

  int mode_icon_size=32;

  btn_pause_refresh=new QToolButton();
  btn_pause_refresh->setIcon(QIcon(":/icons/view-refresh.png"));
  btn_pause_refresh->setIconSize(QSize(mode_icon_size,mode_icon_size));
  btn_pause_refresh->setEnabled(false);
  connect(btn_pause_refresh,SIGNAL(clicked()),this,SLOT(slotPauseRefresh()));
  btn_mode_record=new QToolButton();
  btn_mode_pause=new QToolButton();
  btn_mode_off=new QToolButton();

  btn_mode_record->setIcon(QIcon(":/icons/media-record.png"));
  btn_mode_pause->setIcon(QIcon(":/icons/media-playback-pause.png"));
  btn_mode_off->setIcon(QIcon(":/icons/media-playback-start.png"));

  btn_mode_record->setCheckable(true);
  btn_mode_pause->setCheckable(true);
  btn_mode_off->setCheckable(true);
  
  btn_mode_record->setAutoExclusive(true);
  btn_mode_pause->setAutoExclusive(true);
  btn_mode_off->setAutoExclusive(true);
    
  btn_mode_off->setChecked(true);
  btn_mode_record->setIconSize(QSize(mode_icon_size,mode_icon_size));
  btn_mode_pause->setIconSize(QSize(mode_icon_size,mode_icon_size));
  btn_mode_off->setIconSize(QSize(mode_icon_size,mode_icon_size));

  connect(btn_mode_record,SIGNAL(toggled(bool)),this,SLOT(slotModeToggled()));
  connect(btn_mode_pause,SIGNAL(toggled(bool)),this,SLOT(slotModeToggled()));
  connect(btn_mode_off,SIGNAL(toggled(bool)),this,SLOT(slotModeToggled()));

  layout_mode = new QHBoxLayout();
  layout_mode->addWidget(btn_mode_record);
  layout_mode->addWidget(btn_mode_pause);
  layout_mode->addWidget(btn_pause_refresh);
  layout_mode->addWidget(btn_mode_off);

  layout_mode->addStretch();
  box_mode = new QGroupBox();
  box_mode ->setLayout(layout_mode);
  
  layout_main->addWidget(box_mode);
  layout_main->addStretch();
  this->setLayout(layout_main);


}

void PluginDVRWidget::slotModeToggled() {
  btn_pause_refresh->setEnabled(btn_mode_pause->isChecked());
}

void PluginDVRWidget::slotPauseRefresh() {
  
}

PluginDVR::PluginDVR(FrameBuffer * fb)
 : VisionPlugin(fb)
{
  w=new PluginDVRWidget();
 _settings = new VarList("DVR Settings");
}

PluginDVR::~PluginDVR()
{
  delete _settings;
  delete w;
}

VarList * PluginDVR::getSettings() {
  return _settings;
}

string PluginDVR::getName() {
  return "DVR";
}

ProcessResult PluginDVR::process(FrameData * data, RenderOptions * options) {
  (void)data;
  (void)options;

  //capture data:

  //move stream:

  //output data:

  return ProcessingOk;
}

QWidget * PluginDVR::getControlWidget() {
  return w;
}
