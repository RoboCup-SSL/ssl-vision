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
  \file    VarInt.h
  \brief   C++ Interface: VarInt
  \author  Stefan Zickler, (C) 2008
*/

#ifndef VINT_H_
#define VINT_H_
#include "primitives/VarData.h"
#include <QSpinBox>

/*!
  \class  VarInt
  \brief  A Vartype for storing integers
  \author Stefan Zickler, (C) 2008
  \see    VarTypes.h

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/

class VarInt : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:

  int _val;
  int _def;
  int _min;
  int _max;

public:
  /// get the minimum accepted value if this data-type has a limited range
  /// \see setMin(...)
  /// \see hasMin()
  /// \see unsetMin()
  virtual int getMin() const { DT_LOCK; int v=_min; DT_UNLOCK; return v; }

  /// get the maximum accepted value if this data-type has a limited range
  /// \see setMax(...)
  /// \see hasMax()
  /// \see unsetMax()
  virtual int getMax() const { DT_LOCK; int v=_max; DT_UNLOCK; return v; }

  /// check whether this data type has a limited minimum value
  /// \see getMin(...)
  /// \see setMin(...)
  /// \see unsetMin()
  virtual bool hasMin() const { DT_LOCK; bool v= (_min != INT_MIN); DT_UNLOCK; return v; }

  /// check whether this data type has a limited maximum value
  /// \see getMax(...)
  /// \see setMax(...)
  /// \see unsetMax()
  virtual bool hasMax() const { DT_LOCK; bool v= (_max != INT_MAX); DT_UNLOCK; return v; }

  /// limit the minumum value of this parameter to a pre-set value
  /// \see getMin(...)
  /// \see hasMin()
  /// \see unsetMin()
  virtual void setMin(int minval) { DT_LOCK; _min=minval; DT_UNLOCK; CHANGE_MACRO; }

  /// limit the maximum value of this parameter to a pre-set value
  /// \see getMax(...)
  /// \see hasMax()
  /// \see unsetMax()
  virtual void setMax(int maxval) { DT_LOCK; _max=maxval; DT_UNLOCK; CHANGE_MACRO; }

  /// unset any previous limit of minimum value of this parameter
  /// \see getMin(...)
  /// \see setMin(...)
  /// \see hasMin()
  virtual void unsetMin() { DT_LOCK; _min=INT_MIN; DT_UNLOCK; CHANGE_MACRO; }

  /// unset any previous limit of maximum value of this parameter
  /// \see getMax(...)
  /// \see setMax(...)
  /// \see hasMax()
  virtual void unsetMax() { DT_LOCK; _max=INT_MAX; DT_UNLOCK; CHANGE_MACRO; }

  /// set the value of this node to val.
  virtual bool setInt(int val)
  {
    int tmp;
    if (hasMin() && val < getMin()) {
      tmp=_min;
    } else if (hasMax() && val > getMax()) {
      tmp=_max;
    } else {
      tmp=val;
    }
    DT_LOCK;
    if (tmp!=_val) {
      _val=tmp;
      DT_UNLOCK;
      CHANGE_MACRO;
      return true;
    }
    DT_UNLOCK;
    return false;
  };

  virtual void setDefault(int val)
  {
    DT_LOCK;
    _def=val;
    DT_UNLOCK;
    CHANGE_MACRO;
  }

  VarInt(string name="", int default_val=0, int min_val=INT_MIN, int max_val=INT_MAX) : VarData(name)
  {
    setMin(min_val);
    setMax(max_val);
    DT_LOCK;
    _def=default_val;
    DT_UNLOCK;
    setInt(default_val);
  }

  virtual ~VarInt() {}

  virtual void resetToDefault()
  {
    DT_LOCK;
    int tmp=_def;
    DT_UNLOCK;
    setInt(tmp);
  }

  virtual void printdebug() const
  {
    printf("%d\n",_val);
  }

  virtual vDataTypeEnum getType() const { return DT_INT; };

  virtual string getString() const
  {
    char result[255];
    result[0]=0;
    sprintf(result,"%d",getInt());
    return result;
  };

  virtual int    getInt()    const{ int res; DT_LOCK; res=_val; DT_UNLOCK; return res; };
  virtual double getDouble() const { return (double)getInt(); };
  virtual int 	get() const { return getInt(); };

  virtual bool   getBool()  const { return (getInt() == 1 ? true : false); };

  virtual bool setString(const string & val) { int num=0; sscanf(val.c_str(),"%d",&num); return setInt(num); };
  virtual bool setDouble(double val) { return setInt((int)val);  };
  virtual bool setBool(bool val)     { return setInt(val ? 1 : 0);  };

  //plotting functions:
  virtual bool hasValue() const { return true; }
  virtual bool hasMaxValue() const { return hasMax(); }
  virtual bool hasMinValue() const { return hasMin(); }
  virtual double getMinValue() const { return (double)getMin(); }
  virtual double getMaxValue() const  { return (double)getMax(); }
  virtual double getValue() const { return getDouble(); }


#ifndef VDATA_NO_XML
protected:
  virtual void updateAttributes(XMLNode & us) const
  {
    if (hasMin()) {
      us.updateAttribute(intToString(getMin()).c_str(),"minval","minval");
    } else {
      us.updateAttribute("","minval","minval");
    }
    if (hasMax()) {
      us.updateAttribute(intToString(getMax()).c_str(),"maxval","maxval");
    } else {
      us.updateAttribute("","maxval","maxval");
    }
  }

  virtual void readAttributes(XMLNode & us)
  {
    string s = fixString(us.getAttribute("minval"));
    if (s=="") {
      unsetMin();
    } else {
      int num=0; sscanf(s.c_str(),"%d",&num);
      setMin(num);
    }

    s = fixString(us.getAttribute("maxval"));
    if (s=="") {
      unsetMax();
    } else {
      int num=0; sscanf(s.c_str(),"%d",&num);
      setMax(num);
    }
  }

#endif

//Qt model/view gui stuff:
public:
virtual QWidget * createEditor(const VarItemDelegate * delegate, QWidget *parent, const QStyleOptionViewItem &option) {
  //TODO: hookup editor changes on press-enter and on spin:
  (void)delegate;
  (void)parent;
  (void)option;
  QSpinBox * w = new QSpinBox(parent);
  //connect((const QObject *)w,SIGNAL(editingFinished ( int )),(const QObject *)delegate,SLOT(editorChangeEvent()));
  //uncomment the following line for instantaneous updates:
  //connect((const QObject *)w,SIGNAL(valueChanged ( int )),(const QObject *)delegate,SLOT(editorChangeEvent()));
  return w;
}

virtual void setEditorData(const VarItemDelegate * delegate, QWidget *editor) const {
  (void)delegate;
  QSpinBox * spin=(QSpinBox *) editor;
  spin->setRange(getMin(),getMax() );
  spin->setValue(getInt());
}

virtual void setModelData(const VarItemDelegate * delegate, QWidget *editor) {
  (void)delegate;
  QSpinBox * spin=(QSpinBox *) editor;
  if (setInt(spin->value()) ) mvcEditCompleted();
}




};



#endif /*VINTEGER_H_*/
