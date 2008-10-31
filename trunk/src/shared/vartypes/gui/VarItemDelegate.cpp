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
  \file    VarItemDelegate.cpp
  \brief   C++ Implementation: VarItemDelegate
  \author  Stefan Zickler, (C) 2008
*/

#include "VarItemDelegate.h"

VarItemDelegate::VarItemDelegate(QObject *parent) : QItemDelegate(parent)
{
}

VarItemDelegate::~VarItemDelegate()
{
}

void VarItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {


  QItemDelegate::paint(painter,option,index);
  drawBar(painter,option,index);
}


void VarItemDelegate::drawBar ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {

  //  QItemDelegate::drawBackground(painter,option,index);
  painter->save();  

  if (index.isValid() && index.model()!=0) {
    VarItem * item=(VarItem*)(((VarTreeModel*)index.model())->itemFromIndex (index));
    if (item!=0) {
      if ((item->getColFlags() & (DT_COL_RANGEBARS)) != 0) {
        VarData * dt=item->getVarData();
        if (dt!=0) {
          if (dt->getType() != DT_BOOL && dt->hasValue() && dt->hasMinValue() && dt->hasMaxValue()) {

            QRectF rect=option.rect;
            double border_x=2.0;
            double width=rect.width()-(border_x*2.0);
            if (width < 0.0) width=0.0;
            rect.setX(rect.x()+border_x);
            rect.setWidth(width);

            double range=dt->getMaxValue() - dt->getMinValue();
            double value=dt->getValue()-dt->getMinValue();
            double pos=0.0;
            if (range!=0.0) pos=(value/range)*width;
            if (pos < 0.0) pos=0.0;
            if (pos > width) pos=width;

            QRectF bar=rect;
            double height=2;
            bar.setY(bar.y()+(bar.height() - (height + 1)));
            bar.setHeight(height);
            QRectF frame=bar;
            bar.setWidth(pos);

            QColor high_c;
            if (option.state & QStyle::State_Selected) {
              high_c=option.palette.color(QPalette::HighlightedText);
            } else {
              high_c=option.palette.color(QPalette::Highlight);
            }
            high_c.setAlpha(96);
            //painter->setPen(QPen(high_c,1));
            //painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(high_c));
            painter->drawRect(frame);

            high_c.setAlpha(192);
            QColor nocolor(255,255,255,0) ;
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(high_c));
            painter->drawRect(bar);

          }
        }
      }
    }
  }
  painter->restore();
}



QWidget * VarItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const {
  (void)option;
  if (index.isValid() && index.model()!=0) {
    VarItem * item=(VarItem*)(((VarTreeModel*)index.model())->itemFromIndex (index));
    if (item!=0) {
      VarData * dt=item->getVarData();
      if (dt!=0) {
        if (dt->getType()==DT_BOOL) {
          QComboBox * combo=new QComboBox(parent);
          combo->addItem("true");
          combo->addItem("false");
          return combo;
        } else if (dt->getType()==DT_INT) {
          return new QSpinBox(parent);   
        } else if (dt->getType()==DT_DOUBLE) {
          return new QDoubleSpinBox(parent);
        } else if ((dt->getType()==DT_QWIDGET) || (dt->getType()==DT_TRIGGER)) {
          QWidget * qw = ((VarQWidget*)dt)->createQWidget();
          if (qw!=0) {
            qw->setParent(parent);
          }
          return qw;
        } else if (dt->getType()==DT_STRINGENUM) {
          return new QComboBox(parent);
        } else if (dt->getType()==DT_LIST ||
                   dt->getType()==DT_BLOB ||
                   dt->getType()==DT_UNDEFINED ||
                   dt->getType()==DT_EXTERNAL) {
          return 0;
        } else {
          return new QLineEdit(parent);
          //return QItemDelegate::createEditor(parent,option,index);
        }
      }
    } 
  }
  return 0;
}

void VarItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (editor==0) return;
  if (index.isValid() && index.model()!=0) {
    VarItem * item=(VarItem*)(((VarTreeModel*)index.model())->itemFromIndex (index));
    if (item!=0) {
      VarData * dt=item->getVarData();
      if (dt!=0) {
        if ((dt->getRenderFlags() & DT_FLAG_READONLY) != 0x00) {
          editor->setEnabled(false);
        } else {
          editor->setEnabled(true);
        }
        if (dt->getType()==DT_BOOL) {
          QComboBox * combo=(QComboBox *) editor;
          if (((VarBool *)dt)->getBool()==true) {
            combo->setCurrentIndex(0);
          } else {
            combo->setCurrentIndex(1);
          }
          return;
        } else if (dt->getType()==DT_INT) {
          QSpinBox * spin=(QSpinBox *) editor;
          spin->setRange(((VarInt *)dt)->getMin(),((VarInt *)dt)->getMax() );
          spin->setValue(((VarInt *)dt)->getInt());
        } else if (dt->getType()==DT_DOUBLE) {
          QDoubleSpinBox * spin=(QDoubleSpinBox *) editor;
          spin->setDecimals(10);
          spin->setRange(((VarDouble *)dt)->getMin(),((VarDouble *)dt)->getMax() );
          spin->setValue(((VarDouble *)dt)->getDouble());
        } else if (dt->getType()==DT_STRINGENUM) {
          QComboBox * combo =(QComboBox *)editor;
          int n = ((VarStringEnum *)dt)->getCount();
          QString tmp;
          combo->clear();
          for (int i=0;i<n;i++) {
            tmp=QString::fromStdString(((VarStringEnum *)dt)->getLabel(i));
            combo->insertItem(combo->count(), tmp);
            if (tmp==QString::fromStdString(((VarStringEnum *)dt)->getString())) {
              combo->setCurrentIndex(i);
            }
          }
        } else if ((dt->getType()==DT_QWIDGET) || (dt->getType()==DT_TRIGGER)) {
          ((VarQWidget*)dt)->setEditorData(editor,index);
        } else {
          QLineEdit * ledit=(QLineEdit *) editor;
          ledit->setText(QString::fromStdString(dt->getString()));
        }
      }
    } 
  }
  QItemDelegate::setEditorData(editor,index);
}

void VarItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const {
  if (editor==0) return;
  if (index.isValid() && model!=0) {
    VarItem * item=(VarItem*)(((VarTreeModel*)model)->itemFromIndex (index));
    if (item!=0) {
      VarData * dt=item->getVarData();
      if (dt!=0) {
        if (dt->getType()==DT_BOOL) {
          QComboBox * combo=(QComboBox *) editor;
          if (((VarBool *)dt)->setBool(combo->currentIndex()==0)) dt->mvcEditCompleted();
        } else if (dt->getType()==DT_INT) {
          QSpinBox * spin=(QSpinBox *) editor;
          if (((VarInt *)dt)->setInt(spin->value()) ) dt->mvcEditCompleted();
        } else if (dt->getType()==DT_DOUBLE) {
          QDoubleSpinBox * spin=(QDoubleSpinBox *) editor;
          if (((VarDouble *)dt)->setDouble(spin->value())) dt->mvcEditCompleted();
        } else if (dt->getType()==DT_STRINGENUM) {
          QComboBox * combo=(QComboBox *) editor;
          if (((VarStringEnum *)dt)->select(combo->currentText().toStdString())) dt->mvcEditCompleted();
        } else if ((dt->getType()==DT_QWIDGET) || (dt->getType()==DT_TRIGGER)) {
          ((VarQWidget*)dt)->getEditorData(editor,index);
        } else {
          QLineEdit * ledit=(QLineEdit *) editor;
          if (dt->setString(ledit->text().toStdString())) dt->mvcEditCompleted();
        }
        return;
      }
    } 
  }
  QItemDelegate::setModelData(editor,model,index);
}

void VarItemDelegate::updateEditorGeometry(QWidget *editor,
      const QStyleOptionViewItem &option, const QModelIndex &index) const {
  QItemDelegate::updateEditorGeometry(editor,option,index);
}

