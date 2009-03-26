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
  \file    VarTypes.cpp
  \brief   Static constructor of the VarTypes system
  \author  Stefan Zickler, (C) 2008
*/
#include "VarTypes.h"

VarData * VarData::newVarType(vDataTypeEnum t)
{
  if (t==DT_BOOL) {
    return new VarBool();
  } else if (t==DT_INT) {
    return new VarInt();
  } else if (t==DT_DOUBLE) {
    return new VarDouble();
  } else if (t==DT_STRING) {
    return new VarString();
  } else if (t==DT_BLOB) {
    return new VarBlob();
  } else if (t==DT_EXTERNAL) {
    return new VarExternal();
    /*} else if (t==DT_VECTOR2D) {
    	//FIXME: implement vec2d
    } else if (t==DT_VECTOR3D) {
    	//FIXME: implement vec3d*/
  #ifdef DT_USE_TYPES_TIMEBASED
    } else if (t==DT_TIMEVAR) {
      return new TimeVar();
    } else if (t==DT_TIMELINE) {
      return new TimeLine();
  #endif
  } else if (t==DT_LIST) {
    return new VarList();
  } else if (t==DT_STRINGENUM) {
    return new VarStringEnum();
  } else if (t==DT_SELECTION) {
    return new VarSelection();
  #ifndef VDATA_NO_QT
  } else if (t==DT_QWIDGET) {
    return new VarQWidget();
  } else if (t==DT_TRIGGER) {
    return new VarTrigger();
  #endif
  } else {
    return new VarData();
  }
}
