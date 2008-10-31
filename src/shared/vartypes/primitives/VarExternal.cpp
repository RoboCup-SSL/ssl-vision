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
  \file    VarExternal.cpp
  \brief   C++ Implementation: VarExternal
  \author  Stefan Zickler, (C) 2008
*/

#include "primitives/VarExternal.h"

VarExternal::VarExternal(string _filename, VarList * vlist) : VarList(vlist->getName())
{
  list=vlist->getChildren();
  DT_LOCK;
  filename=_filename;
  DT_UNLOCK;
}

VarExternal::VarExternal(string _filename, string _name) : VarList(_name)
{
  DT_LOCK;
  filename=_filename;
  DT_UNLOCK;
}

VarExternal::~VarExternal()
{
}
