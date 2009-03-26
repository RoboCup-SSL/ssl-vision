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
  \file    VarTypes.h
  \brief   The main header for the VarTypes system
  \author  Stefan Zickler, (C) 2008

  This is the main header for the VarTypes system.
  It provides definitions for all base-types.

  \b About:     VarTypes are an object-oriented, abstracted
                framework for handling variables of all primitive, but
                also arbitrarily complex types.

  \b Features:  VarTypes allow hierarchical orginazation, thread-safety,
                load/store to/from xml, optional QT4-based signals,
                and optional QT4-based model/view visualization.
                Supported base types are int, bool, double, string,
                lists, binary data (uchar* blocks).
                More complex types can be added easily by inheriting the
                VarData base-class.

  \b Usage:     All VarTypes extend the base-class VarData (see \c VarData.h )
*/
//========================================================================

#ifndef VTYPES_H_
#define VTYPES_H_

//include base-class
#include "primitives/VarData.h"

//include primitives
#include "primitives/VarInt.h"
#include "primitives/VarBool.h"
#include "primitives/VarDouble.h"
#include "primitives/VarString.h"
#include "primitives/VarBlob.h"
#include "primitives/VarList.h"
#include "primitives/VarStringEnum.h"
#include "primitives/VarSelection.h"
#include "primitives/VarExternal.h"
#include "primitives/VarQWidget.h"
#include "primitives/VarTrigger.h"

#ifdef DT_USE_TYPES_TIMEBASED
  #include "enhanced/TimeLine.h"
  #include "enhanced/TimeVar.h"
#endif

#endif /*VTYPES_H_*/
