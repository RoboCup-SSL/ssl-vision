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
  \file    TimeVar.h
  \brief   C++ Interface: TimeVar
  \author  Stefan Zickler, (C) 2008
*/

#ifndef TIMEVAR_H_
#define TIMEVAR_H_
#include "primitives/VarData.h"
#include "TimeIndex.h"

class TimeVar : public VarData, public TimeIndex {
    Q_OBJECT
  protected:
    VarData * data;
  public:
    TimeVar ( string _name = "", TimeIndex idx = 0.0, VarData * vdt = 0 ) : VarData ( _name ), TimeIndex ( idx ) {
      setData ( vdt );
    };
    VarData * getData() const {
      return data;
    }
    void setData ( VarData * vdt ) {
      data = vdt;
    }
    virtual ~TimeVar() {};
    virtual vDataTypeEnum getType() const { return DT_TIMEVAR; };
    virtual string getTypeName() const { return typeToString ( getType() ); };
    virtual string getString() const {
      char result[255];
      result[0] = 0;
      sprintf ( result, "%lf", getTime() );
      return result;
    };
    virtual void setString ( string val ) { double num = 0; sscanf ( val.c_str(), "%lf", &num ); setTime ( num ); };


    virtual void resetToDefault() {
      setTime ( 0.0 );
      CHANGE_MACRO;
    }
    virtual void printdebug() const {
      printf ( "Time Index : %f\n", getTime() );
    }

    virtual void setDouble ( double val ) { setTime ( val ); CHANGE_MACRO; };


    virtual double getDouble() const { return getTime(); }

    virtual bool hasValue() const { return true; }
    virtual double getValue() const { return getTime(); }
    virtual bool hasMaxValue() const { return false; }
    virtual bool hasMinValue() const { return false; }

    virtual vector<VarData *> getChildren() const {
      vector<VarData *> v;
      v.push_back ( data );
      return v;
    }

    virtual void setSerialString ( string val ) {

    }

    virtual string getSerialString() const {
      return "";
    }


#ifndef VDATA_NO_XML
  protected:

    virtual void readChildren ( XMLNode & us ) {
      vector<VarData *> v;
      v = readChildrenHelper ( us, v, false, true );
      if ( v.size() > 0 ) {
        data = v[0];
      } else {
        data = 0;
      }
      CHANGE_MACRO;
    }


    virtual void readAttributes ( XMLNode & us ) {
      string s = fixString ( us.getAttribute ( "time" ) );
      if ( s == "" ) {
        setTime ( 0.0 );
      } else {
        double num = 0; sscanf ( s.c_str(), "%lf", &num );
        setTime ( num );
      }
      CHANGE_MACRO;
    }

    virtual void updateAttributes ( XMLNode & us ) const {
      us.updateAttribute ( doubleToString ( getTime() ).c_str(), "time", "time" );
    }

#endif





};



#endif /*TIMEVAR_H_*/
