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
  \file    ValueVis.cpp
  \brief   C++ Implementation: ValueVis
  \author  Stefan Zickler, (C) 2008
*/
#include "ValueVis.h"

ValueVis::ValueVis(VarType * _dt)
{
  dt=_dt;
  children = new QList<ValueVis *>();
  setupUi();
  update();
}

void ValueVis::setupUi() {
  QVBoxLayout * layout = new QVBoxLayout();
  layout->setMargin(2);
  layout->setSpacing(2);
  title = new QLabel();
  layout->addWidget(title);
  setFrameStyle(QFrame::StyledPanel);
  layout->setSizeConstraint(QLayout::SetMinimumSize);

  setLayout(layout);
  if (dt!=0) {
    if (dt->getType() == DT_LIST) {
      vector<VarType *> v = dt->getChildren();
      for (unsigned int i=0;i<v.size();i++) {
        children->append(new ValueVis(v[i]));
        layout->addWidget(children->last());
      }
    } else {
      label = new QLabel();
      label->setAlignment(Qt::AlignRight);
      layout->addWidget(label);
    }
  }
}


ValueVis::~ValueVis()
{
	
}

void ValueVis::update(bool recurse) {
  QFont font=title->font();
  font.setPointSize(8);
  title->setFont(font);

//  this->setVisible(dt->areFlagsSet( VARTYPE_FLAG_HIDDEN));

  
  if (dt!=0) {
    title->setText(QString::fromStdString(dt->getName()));
    if (dt->getType() == DT_LIST) {
      if (recurse) {
        /*vector<VarType *> v = dt->getChildren();
        for (unsigned int i=0;i<v.size();i++) {
          layout->addWidget(new ValueVis(v[i]));
        }*/
        
        //for (int i=0;i<children.size();i++) {
        //  children[i]->update(recurse);
        //}
      }
    } else {
      font=label->font();
      font.setPointSize(9);
      font.setBold(true);
      label->setFont(font);
      label->setText(QString::fromStdString(dt->getString()));
    }
  } else {
    title->setText("no title");
  }
}
