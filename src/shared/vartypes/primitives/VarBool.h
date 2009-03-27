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
  \file    VarBool.h
  \brief   C++ Interface: VarBool
  \author  Stefan Zickler, (C) 2008
*/

#ifndef VARBOOL_H_
#define VARBOOL_H_
#include "primitives/VarData.h"
#include <QCheckBox>
/*!
  \class  VarBool
  \brief  A Vartype for storing booleans
  \author Stefan Zickler, (C) 2008
  \see    VarTypes.h

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/

class VarBool : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:

  bool _val;
  bool _def;
public:

  VarBool(string _name="", bool default_val=false) : VarData (_name)
  {
    DT_LOCK;
    _val=_def=default_val;
    _flags |= DT_FLAG_PERSISTENT;
    DT_UNLOCK;
    CHANGE_MACRO;
  }

  virtual ~VarBool() {}

  virtual void resetToDefault()
  {
    DT_LOCK;
    _val=_def;
    DT_UNLOCK;
    CHANGE_MACRO;
  }

  virtual void setDefault(bool val)
  {
    DT_LOCK;
    _def=val;
    DT_UNLOCK;
    CHANGE_MACRO;
  }

  virtual void printdebug() const
  {
    DT_LOCK;
    printf("%s\n",(_val ? "true" : "false"));
    DT_UNLOCK;
  }

  virtual vDataTypeEnum getType() const { return DT_BOOL; };

  /// will return the string "true" if true, or "false" if false.
  virtual string getString() const { return (getBool() ? "true" : "false"); };
  /// will return 1.0 if true, 0.0 if false.
  virtual double getDouble()const { return (getBool() ? 1.0 : 0.0); };
  /// will return 1 if true, 0 if false
  virtual int    getInt()   const { return (getBool() ? 1 : 0); };
  /// return the boolean value
  virtual bool   getBool() const  { DT_LOCK; bool v=_val; DT_UNLOCK; return v; };

  /// will set this to true if the string is "true" or false otherwise
  virtual bool setString(const string & val) { return setBool(val=="true"); };
  /// will set this to true if the value is 1.0 or false otherwise
  virtual bool setDouble(double val) { return setBool(val==1.0); };
  /// will set this to true if the value is 1 or false otherwise
  virtual bool setInt(int val)       { return setBool(val==1); };
  /// set this to a particular boolean value
  virtual bool setBool(bool val)     { DT_LOCK; if (val!=_val) { _val=val; DT_UNLOCK; CHANGE_MACRO; return true;} else { DT_UNLOCK; return false; }; };

  //plotting functions:
  virtual bool hasValue() const { return true; }
  virtual bool hasMaxValue() const { return true; }
  virtual bool hasMinValue() const { return true; }
  virtual double getMinValue() const { return 0.0; }
  virtual double getMaxValue() const  { return 1.0; }
  virtual double getValue() const { return getDouble(); }

  //Qt model/view gui stuff:
  virtual QWidget * createEditor(const VarItemDelegate * delegate, QWidget *parent, const QStyleOptionViewItem &option) {
    (void)option;
    QCheckBox * checker=new QCheckBox(parent);
    connect((const QObject *)checker,SIGNAL(stateChanged(int)),(const QObject *)delegate,SLOT(editorChangeEvent()));
    if (getBool()) {
      checker->setText("True"); 
    } else {
      checker->setText("False");
    }
    return checker;

  }
  virtual void setEditorData(const VarItemDelegate * delegate, QWidget *editor) const {
    (void)delegate;
    QCheckBox * checker=(QCheckBox *) editor;
    checker->setChecked(getBool());

  }
  virtual void setModelData(const VarItemDelegate * delegate, QWidget *editor) {
    (void)delegate;
    QCheckBox * checker=(QCheckBox *) editor;
    if (checker->isChecked()) {
      checker->setText("True"); 
    } else {
      checker->setText("False");
    }
    if (setBool(checker->isChecked())) mvcEditCompleted();

  }

};
#endif /*VBOOL_H_*/
