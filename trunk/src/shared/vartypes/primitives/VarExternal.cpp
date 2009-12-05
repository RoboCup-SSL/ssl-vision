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

namespace VarTypes {

VarExternal::VarExternal(string _filename, VarList * vlist) : VarList(vlist->getName())
{
  list=vlist->getChildren();
  lock();
  filename=_filename;
  unlock();
}

VarExternal::VarExternal(string _filename, string _name) : VarList(_name)
{
  lock();
  filename=_filename;
  unlock();
}

VarExternal::~VarExternal()
{
}

void VarExternal::loadExternal() {
  lock();
  //load file to empty parent
  int before=list.size();
  XMLNode parent = XMLNode::openFileHelper(filename.c_str(),"VarXML");
  list=readChildrenHelper(parent, list, false, false);
  int after=list.size();
  if (after > before) {
    for (int i=before; i < after; i++) {
        emit(childAdded(list[i]));
    }
  }
  changed();
  unlock();
}

void VarExternal::readChildren(XMLNode & us) {
  (void)us;
  loadExternal();
}
};
