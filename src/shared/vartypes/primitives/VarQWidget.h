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
  \file    VarQWidget.h
  \brief   C++ Interface: VarInt
  \author  Stefan Zickler, (C) 2008
*/


#ifndef VARQWIDGET_H_
#define VARQWIDGET_H_
#include "primitives/VarData.h"
#include <QWidget>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QPushButton>

/*!
  \class  VarQWidget
  \brief  An Vartype for embedding QWidgets into the Var-tree
  \author Stefan Zickler, (C) 2008
  \see    VarQWidget
  \see    VarTypes.h

  Note, that this class breaks the model/view paradigm as it
  will construct a single qwidget which will be shared between viewers
  and not separate instances for each viewer. This is fine if you
  only have a single viewer, but will likely create problems if you have
  multiple ones.

  To avoid these issues, it is better to simply inherit VarData and
  create a qwidget factory by reimplementing VarData's
  createEditor(...);

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/


class VarQWidget : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:

  QWidget * _val;
public:

  VarQWidget(string _name="", QWidget * default_val=0) : VarData (_name)
  {
    _val=default_val;
    _flags |= DT_FLAG_PERSISTENT;
    CHANGE_MACRO;
  }

  virtual ~VarQWidget() {
    if (_val!=0) delete _val;
  }

  virtual void resetToDefault()
  {
  }

  virtual void printdebug() const
  {
    printf("QWidget pointer: %p\n",_val);
  }

  virtual vDataTypeEnum getType() const { return DT_QWIDGET; };

  virtual QWidget * getQWidget() const  { 
    return _val;
  };

  virtual bool setQWidget(QWidget * val)     { if (val!=_val) { _val=val; CHANGE_MACRO; return true;} else { return false; }; };


//Qt model/view gui stuff:
public:
  virtual QWidget * createEditor(const VarItemDelegate * delegate, QWidget *parent, const QStyleOptionViewItem &option) {
    (void)delegate;
    (void)parent;
    (void)option;
    QWidget * w=getQWidget();
    if (w!=0) w->setParent(parent);
    return w;
  };
  virtual void setEditorData(const VarItemDelegate * delegate, QWidget *editor) const {
    (void)delegate;
    (void)editor;
  };
  virtual void setModelData(const VarItemDelegate * delegate, QWidget *editor) {
    (void)delegate;
    (void)editor;
  }

};
#endif /*VBOOL_H_*/
