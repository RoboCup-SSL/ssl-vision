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
  \class  VarAbstractQWidget
  \brief  An abstract Vartype for embedding QWidgets into the Var-tree
  \author Stefan Zickler, (C) 2008
  \see    VarQWidget
  \see    VarTypes.h

  Note, that in order to obtain a persistent editable item,
  you should rather use \c VarQWidget which extends this class.

  This VarAbstractQWidget is really only useful if you need
  to develop your own special non-persistent VarType.

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/

class VarAbstractQWidget : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
public:
  VarAbstractQWidget(string _name="") : VarData(_name)
  {

  };

  virtual void getEditorData(QWidget * editor, const QModelIndex &index) {
    (void)editor;
    (void)index;
  }

  virtual void setEditorData(QWidget * editor, const QModelIndex &index) {
    (void)editor;
    (void)index;
  }

  virtual QWidget * createQWidget() const  {
    return 0;
  }
};

class VarQWidget : public VarAbstractQWidget
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:

  QWidget * _val;
public:

  VarQWidget(string _name="", QWidget * default_val=0) : VarAbstractQWidget (_name)
  {
    _val=default_val;
    addRenderFlags(DT_FLAG_PERSISTENT);
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

  virtual void getEditorData(QWidget * editor, const QModelIndex &index) {
    (void)editor;
    (void)index;

  }

  virtual void setEditorData(QWidget * editor, const QModelIndex &index) {
    (void)editor;
    (void)index;
    //nothing...
    /*QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(_val);
    editor->setLayout(layout);*/

  }

  virtual vDataTypeEnum getType() const { return DT_QWIDGET; };

  virtual QWidget * getQWidget() const  { 
    return _val;
  };

  virtual bool setQWidget(QWidget * val)     { if (val!=_val) { _val=val; CHANGE_MACRO; return true;} else { return false; }; };

};
#endif /*VBOOL_H_*/
