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
  \file    VarString.h
  \brief   C++ Interface: VarString
  \author  Stefan Zickler, (C) 2008
*/

#ifndef VSTRING_H_
#define VSTRING_H_
#include "primitives/VarData.h"


/*!
  \class  VarString
  \brief  This is the string VarType of the VarTypes system
  \author Stefan Zickler, (C) 2008
  \see    VarTypes.h

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/
class VarString : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:

  string _val;
  string _def;
public:

  VarString(string name="", string default_val="") : VarData(name)
  {
    DT_LOCK;
    _val=_def=default_val;
    DT_UNLOCK;
    CHANGE_MACRO;
  }

  virtual ~VarString() {}

  virtual void resetToDefault()
  {
    DT_LOCK;
    _val=_def;
    DT_UNLOCK;
    CHANGE_MACRO;
  }
  virtual void setDefault(string val)
  {
    DT_LOCK;
    _def=val;
    DT_UNLOCK;
    CHANGE_MACRO;
  }
  virtual void printdebug() const
  {
    DT_LOCK;
    printf("%s\n",_val.c_str());
    DT_UNLOCK;
  }

  virtual vDataTypeEnum getType() const { return DT_STRING; };
  virtual string getString() const { DT_LOCK; string v=_val; DT_UNLOCK; return v;  };
  virtual bool   hasValue()  const { return false; };
  virtual bool setString(const string & val) { DT_LOCK; if (_val!=val) {_val=val; DT_UNLOCK; CHANGE_MACRO; return true;} else { DT_UNLOCK; return false;} };


};

#endif /*VSTRING_H_*/
