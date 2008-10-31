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
  \file    VarStringEnum.cpp
  \brief   C++ Implementation: VarStringEnum
  \author  Stefan Zickler, (C) 2008
*/

#include "primitives/VarStringEnum.h"

VarStringEnum::VarStringEnum(string _name, string _default) : VarData(_name) {
  default_string=_default;
  selected=_default;
}

VarStringEnum::~VarStringEnum() {
  int n = list.size();
  for (int i=n-1;i>=0;i--) {
    delete list[(unsigned int)i];
  }
}
