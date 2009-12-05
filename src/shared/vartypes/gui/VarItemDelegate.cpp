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
namespace VarTypes {
  VarItemDelegate::VarItemDelegate(QObject *parent) : QItemDelegate(parent)
  {
  }
  
  VarItemDelegate::~VarItemDelegate()
  {
  }
  
  void VarItemDelegate::editorChangeEvent() {
    emit(commitData((QWidget *)sender()));
  }
  
  void VarItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
  
  
    if (index.isValid() && index.model()!=0) {
      VarItem * item=(VarItem*)(((VarTreeModel*)index.model())->itemFromIndex (index));
      if (item!=0) {
        VarType * dt=item->getVarType();
        if (dt!=0) {
          /*if ((dt->getFlags() & VARTYPE_FLAG_PERSISTENT) != 0x00) {
            //there's a persistent editor on top!
            //don't draw anything!
            return;
          }*/
          dt->paint(this,painter,option,index);
          if ((item->getColFlags() & (GUI_COLUMN_FLAG_RANGEBARS)) != 0) {
            drawBar(dt,painter,option,index);
          }
          return;
        }
      }
    }
    QItemDelegate::paint(painter,option,index);
  }
  
  
  void VarItemDelegate::drawBar (VarType * dt, QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
    (void)index;
    //  QItemDelegate::drawBackground(painter,option,index);
  
    if (dt!=0) {
      if (dt->getType() != VARTYPE_ID_BOOL && dt->hasValue() && dt->hasMinValue() && dt->hasMaxValue()) {
        painter->save();  
    
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
  
        painter->restore();
      }
    }
  }
  
  
  
  QWidget * VarItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
    (void)option;
    if (index.isValid() && index.model()!=0) {
      VarItem * item=(VarItem*)(((VarTreeModel*)index.model())->itemFromIndex (index));
          if (item!=0) {
        VarType * dt=item->getVarType();
        if (dt!=0) {
          QWidget * w;
          w=dt->createEditor(this,parent,option);
          //if ((dt->getFlags() & VARTYPE_FLAG_PERSISTENT) != 0x00) w->setFocusPolicy(Qt::NoFocus);
          return w;
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
        VarType * dt=item->getVarType();
        if (dt!=0) {
          //printf("Setting Editor for: %s\n",dt->getName().c_str());
          if (editor!=0 && ((dt->getFlags() & VARTYPE_FLAG_READONLY) != 0x00)) {
            editor->setEnabled(false);
          } else {
            editor->setEnabled(true);
          }
          dt->setEditorData(this,editor);
          //added on 3/27/09....return if we are done.
          return;
        }
      } 
    }
    QItemDelegate::setEditorData(editor,index);
  }
  
  QSize VarItemDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const {
  
    if (index.isValid() && index.model()!=0) {
      if ((index.row() >= 0) && (index.column() >= 0)) {
        QStandardItem *parent = static_cast<QStandardItem*>(index.internalPointer());
        if (parent != 0) {
          QStandardItem *item = parent->child(index.row(), index.column());
          if (item!=0) {
            VarType * dt=((VarItem*)item)->getVarType();
            if (dt!=0) {
              return dt->sizeHint(this,option,index);
            }
          }
        }
      }
    }
    return QItemDelegate::sizeHint(option,index);
  }
  
  
  void VarItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const {
    if (editor==0) return;
    if (index.isValid() && model!=0) {
      VarItem * item=(VarItem*)(((VarTreeModel*)model)->itemFromIndex (index));
      if (item!=0) {
        VarType * dt=item->getVarType();
        if (dt!=0) {
          dt->setModelData(this,editor);
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
};
