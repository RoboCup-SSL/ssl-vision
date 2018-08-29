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
  \file    VarBase64.cpp
  \brief   C++ Implementation: VarBase64
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "VarBase64.h"

namespace VarTypes {
  VarBase64* VarBase64::pinstance = 0;// initialize pointer
  
  XMLParserBase64Tool* VarBase64::getTool ()
  {
    if (pinstance == 0)  // is it the first call?
    {
      pinstance = new VarBase64; // create sole instance
    }
    return pinstance->tool; // address of sole instance
  }
  
  VarBase64::VarBase64()
  {
    tool = new XMLParserBase64Tool();
  }
};
