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
  \file    neurowidget.cpp
  \brief   C++ Implementation: NeuroWidget
  \author  J. A. Gurzoni Jr, (C) 2012
*/
//========================================================================

#include <QtGui>
#include "neurowidget.h"

NeuroWidget::NeuroWidget(LUT3D * lut, LUTChannelMode mode)
{
  setupUi((QWidget*)this);
  _mode=mode;
  currentChannel = -1;
  pending_loadnn = false;
  pending_train = false;
  pending_train = false;

  //toolbar=new QToolBar();
  //vbox=new QBoxLayout(QBoxLayout::TopToBottom);
  //hbox=new QBoxLayout(QBoxLayout::LeftToRight);

  //btn_train = new QPushButton(tr("Train Neural Network"));
  connect(btn_train, SIGNAL(clicked()), SLOT(is_clicked_btn_train()));
  //btn_loadnn = new QPushButton(tr("Load Neural to LUT"));
  connect(btn_loadnn, SIGNAL(clicked()), SLOT(is_clicked_btn_loadnn()));
  //btn_clear = new QPushButton(tr("Clear Training Set"));
  connect(btn_clear, SIGNAL(clicked()), SLOT(is_clicked_btn_clear()));
  //gllut=new GLLUTWidget(mode,this);
  //gllut->setLUT(lut);
  list=new QListWidget(this);
  list->setGeometry(QRect(50,40,120,200));
  updateList(lut);
  connect(list,SIGNAL(currentRowChanged(int)),this, SLOT(selectChannel(int)));
  list->setFixedWidth(list->sizeHintForColumn ( 0 ) + 5 );
  list->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );
  list->setFocusPolicy(Qt::NoFocus);
  setFocusPolicy(Qt::StrongFocus);
  //gllut->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding );

  //label->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  //toolbar->setIconSize(QSize(16,16));
  //toolbar->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  //vbox->setSpacing(2);
  //vbox->setMargin(0);
  //hbox->setSpacing(2);
  //hbox->setMargin(0);
  ////hbox->addWidget(gllut);
  //hbox->addWidget(list);
  //vbox->addWidget(toolbar);
  //vbox->addLayout(hbox);
  //vbox->addWidget(label);
  //vbox->addWidget(btn_train);
  //vbox->addWidget(btn_loadnn);
  //vbox->addWidget(btn_clear);

  //this->setLayout(vbox);

  //addActions(gllut->actions());

  //toolbar->addActions(actions());
  //toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                //QList<QAction *> toolactions=gllut->actions();
                //reverse the list so the icons appear in the right order:
                // (qt really needs a list.reverse() function....)
                /*for (int i=0;i<toolactions.size()/2;i++) {
                        toolactions.swap(i,toolactions.size()-(i+1));
                }*/
                //add the actions to the toolbar


}


void NeuroWidget::selectChannel(int c) {
  if (c!=(-1)) {
    currentChannel = c;
  }
}

void NeuroWidget::updateList(LUT3D * lut) {
  QListWidgetItem * item;
  for (unsigned int i=0;i<lut->channels.size();i++) {
    item=new QListWidgetItem(QString::fromStdString(lut->channels[i].label));
    QPixmap p(16,16);
    p.fill(QColor(lut->channels[i].draw_color.r,lut->channels[i].draw_color.g,lut->channels[i].draw_color.b));
    QPainter painter(&p);
    painter.setPen(QPen(QColor(0,0,0),1));
    painter.drawRect(0,0,15,15);

    item->setIcon(p);
    list->addItem(item);
  }
}

NeuroWidget::~NeuroWidget()
{
  delete gllut;
  delete label;
  delete list;
  delete hbox;
  delete vbox;
  delete toolbar;
}

void NeuroWidget::focusInEvent ( QFocusEvent * event ) {
  (void)event;
  //forward the focus to the actual widget that we contain
  //gllut->setFocus(Qt::OtherFocusReason);
}


int NeuroWidget::getStateChannel() {
  return currentChannel;
}

void NeuroWidget::is_clicked_btn_loadnn(){
    pending_loadnn = true;
}

void NeuroWidget::is_clicked_btn_train(){
    pending_train = true;
}
void NeuroWidget::is_clicked_btn_clear(){
    pending_clear = true;
}

void NeuroWidget::setStatusLabel(const QString &str_input){
    this->label->setText(str_input);
}

void NeuroWidget::setSamplesLabel(int samples){
    this->lbl_sample->setText("Samples Captured: "+QString::number(samples));
}
