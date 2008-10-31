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
  \file    VarDouble.h
  \brief   C++ Interface: VarDouble
  \author  Stefan Zickler, (C) 2008
*/

#ifndef VDOUBLE_H_
#define VDOUBLE_H_
#include "primitives/VarData.h"

/*!
  \class  VarDouble
  \brief  A Vartype for storing double precision floating points
  \author Stefan Zickler, (C) 2008
  \see    VarTypes.h

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/

class VarDouble : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:
  double _val;
  double _def;
  double _min;
  double _max;
public:
  /// get the minimum accepted value if this data-type has a limited range
  /// \see setMin(...)
  /// \see hasMin()
  /// \see unsetMin()
  virtual double getMin() const { DT_LOCK; double v=_min; DT_UNLOCK; return v; }

  /// get the maximum accepted value if this data-type has a limited range
  /// \see setMax(...)
  /// \see hasMax()
  /// \see unsetMax()
  virtual double getMax() const { DT_LOCK; double v=_max; DT_UNLOCK; return v; }

  /// check whether this data type has a limited minimum value
  /// \see getMin(...)
  /// \see setMin(...)
  /// \see unsetMin()
  virtual bool hasMin() const { DT_LOCK; bool v= (_min != DBL_MIN); DT_UNLOCK; return v; }

  /// check whether this data type has a limited maximum value
  /// \see getMax(...)
  /// \see setMax(...)
  /// \see unsetMax()
  virtual bool hasMax() const { DT_LOCK; bool v= (_max != DBL_MAX); DT_UNLOCK; return v; }

  /// limit the minumum value of this parameter to a pre-set value
  /// \see getMin(...)
  /// \see hasMin()
  /// \see unsetMin()
  virtual void setMin(double minval) { DT_LOCK; _min=minval; DT_UNLOCK; CHANGE_MACRO; }

  /// limit the maximum value of this parameter to a pre-set value
  /// \see getMax(...)
  /// \see hasMax()
  /// \see unsetMax()
  virtual void setMax(double maxval) { DT_LOCK; _max=maxval; DT_UNLOCK; CHANGE_MACRO; }

  /// unset any previous limit of minimum value of this parameter
  /// \see getMin(...)
  /// \see setMin(...)
  /// \see hasMin()
  virtual void unsetMin() { DT_LOCK; _min=DBL_MIN; DT_UNLOCK; CHANGE_MACRO; }

  /// unset any previous limit of maximum value of this parameter
  /// \see getMax(...)
  /// \see setMax(...)
  /// \see hasMax()
  virtual void unsetMax() { DT_LOCK; _max=DBL_MAX; DT_UNLOCK; CHANGE_MACRO; }


  /// set the value of this node to val.
  virtual bool setDouble(double val)
  {
    double tmp;
    if (hasMin() && val < getMin()) {
      tmp=_min;
    } else if (hasMax() &&  val > getMax()) {
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

  VarDouble(string name="", double default_val=0, double min_val=DBL_MIN, double max_val=DBL_MAX) : VarData(name)
  {
    setMin(min_val);
    setMax(max_val);
    DT_LOCK;
    _def=default_val;
    DT_UNLOCK;
    setDouble(default_val);
    CHANGE_MACRO;
  }

  virtual ~VarDouble() {}

  virtual void resetToDefault()
  {
    DT_LOCK;
    double tmp=_def;
    DT_UNLOCK;
    setDouble(tmp);
  }

  virtual void setDefault(double val)
  {
    DT_LOCK;
    _def=val;
    DT_UNLOCK;
    CHANGE_MACRO;
  }
  virtual void printdebug() const
  {
    printf("%f\n",_val);
  }

  virtual vDataTypeEnum getType() const { return DT_DOUBLE; };

  /// get a maximum precision string representation of the current value
  /// Internally this uses sprintf with the %lf argument
  virtual string getString() const
  {
    char result[255];
    result[0]=0;
    sprintf(result,"%lf",getDouble());
    return result;
  };

  /// get the value of this data-type
  virtual double getDouble() const { DT_LOCK; double v=_val; DT_UNLOCK; return v; };

  /// will typecast the value to an int and return it.
  /// Note, that this will not perform any rounding, but rather a standard typecast.
  virtual int    getInt()    const { return (int)getDouble(); };
  /// will return true if the value is 1.0, or false otherwise
  virtual bool   getBool()   const { return (getDouble() == 1.0 ? true : false); };

  /// set this floating point value to some string value.
  /// internally, this uses sscanf with the %lf argument.
  virtual bool setString(const string & val) { double num=0; sscanf(val.c_str(),"%lf",&num); return setDouble(num); };
  virtual bool setInt(int val)       { return setDouble((double)val);  };
  virtual bool setBool(bool val)     { return setDouble(val ? 1.0 : 0.0);  };

  //plotting functions:
  virtual bool hasValue() const { return true; }
  virtual bool hasMaxValue() const { return hasMax(); }
  virtual bool hasMinValue() const { return hasMin(); }
  virtual double getMinValue() const { return getMin(); }
  virtual double getMaxValue() const  { return getMax(); }
  virtual double getValue() const { return getDouble(); }


#ifndef VDATA_NO_XML
protected:
  virtual void updateAttributes(XMLNode & us) const
  {
    if (hasMin()) {
      us.updateAttribute(doubleToString(getMin()).c_str(),"minval","minval");
    } else {
      us.updateAttribute("","minval","minval");
    }
    if (hasMax()) {
      us.updateAttribute(doubleToString(getMax()).c_str(),"maxval","maxval");
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
      double num=0; sscanf(s.c_str(),"%lf",&num);
      setMin(num);
    }

    s = fixString(us.getAttribute("maxval"));
    if (s=="") {
      unsetMax();
    } else {
      double num=0; sscanf(s.c_str(),"%lf",&num);
      setMax(num);
    }
  }

#endif


};


#endif /*VDOUBLE_H_*/
