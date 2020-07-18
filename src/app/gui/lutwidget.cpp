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
  \file    lutwidget.cpp
  \brief   C++ Implementation: LUTWidget
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================


#include "lutwidget.h"
#include <QGroupBox>

LUTWidget::LUTWidget(LUT3D * lut, LUTChannelMode mode)
{
  _mode=mode;
  toolbar=new QToolBar();
  vbox=new QBoxLayout(QBoxLayout::TopToBottom);
  hbox=new QBoxLayout(QBoxLayout::LeftToRight);
  list=new QListWidget();
  gllut=new GLLUTWidget(mode,this);
  gllut->setLUT(lut);
  updateList(lut);
  connect(list,SIGNAL(currentRowChanged(int)),this, SLOT(selectChannel(int)));
  list->setFixedWidth(list->sizeHintForColumn ( 0 ) + 5 );
  list->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );
  list->setFocusPolicy(Qt::NoFocus);
  setFocusPolicy(Qt::StrongFocus);
  gllut->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding );
  toolbar->setIconSize(QSize(16,16));
  toolbar->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

  vbox->setSpacing(2);
  vbox->setMargin(0);
  hbox->setSpacing(2);
  hbox->setMargin(0);
  hbox->addWidget(gllut);
  hbox->addWidget(list);
  vbox->addWidget(toolbar);
  vbox->addLayout(hbox);
  this->setLayout(vbox);

  addActions(gllut->actions());

  toolbar->addActions(actions());
  //toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                //QList<QAction *> toolactions=gllut->actions();
                //reverse the list so the icons appear in the right order:
                // (qt really needs a list.reverse() function....)
                /*for (int i=0;i<toolactions.size()/2;i++) {
                        toolactions.swap(i,toolactions.size()-(i+1));
                }*/
                //add the actions to the toolbar


}


void LUTWidget::selectChannel(int c) {
  if (c!=(-1)) {
    gllut->setChannel(c);
  }
}

void LUTWidget::updateList(LUT3D * lut) {
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

void LUTWidget::samplePixel(const yuv & color) {
  gllut->samplePixel( color );
}

void LUTWidget::add_del_Pixel(yuv color, bool add, bool continuing_undo) {
  gllut->add_del_Pixel(color, add, continuing_undo);
}

void LUTWidget::sampleImage(const RawImage & img) {
  gllut->sampleImage( img );
}

LUTWidget::~LUTWidget()
{
  delete gllut;
  delete list;
  delete hbox;
  delete vbox;
  delete toolbar;
}

void LUTWidget::focusInEvent ( QFocusEvent * event ) {
  (void)event;
  //forward the focus to the actual widget that we contain
  gllut->setFocus(Qt::OtherFocusReason);
}

GLLUTWidget * LUTWidget::getGLLUTWidget() {
  return gllut;
}

/*void LUTWidget::setLUT(LUT3D * lut) {
printf("setting lut\n");fflush(stdout);
printf("myself 2%d\n",this); fflush(stdout);
  printf("gllut: %d\n",gllut); fflush(stdout);
  gllut->setLUT(lut);
}*/

