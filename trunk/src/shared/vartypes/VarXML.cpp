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
  \file    VarXML.cpp
  \brief   C++ Implementation: VarXML
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "VarXML.h"

namespace VarTypes {
  VarXML::VarXML() {};
  VarXML::~VarXML() {};
  
  void VarXML::write(VarType * rootVar, string filename)
  {
    vector<VarType *> v;
    v.push_back(rootVar);
    write(v,filename);
  }
  void VarXML::write(vector<VarType *> rootVars, string filename)
  {
  
    XMLNode root = XMLNode::openFileHelper(filename.c_str(),"VarXML");
    VarType::deleteAllVarChildren(root);
    for (unsigned int i=0;i<rootVars.size();i++) {
      rootVars[i]->writeXML(root,true);
    }
    root.writeToFile(filename.c_str());
  }
  
  vector<VarType *> VarXML::read(vector<VarType *> existing_nodes, string filename)
  {
    XMLNode root = XMLNode::openFileHelper(filename.c_str(),"VarXML");
    return VarType::readChildrenHelper(root ,existing_nodes, false, false);
  }
};
