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
  \file    VarData.cpp
  \brief   C++ Implementation: VarData
  \author  Stefan Zickler, (C) 2008
*/

#include "VarData.h"

vDataTypeEnum VarData::stringToType(string stype)
{
  if (stype=="bool") {
    return DT_BOOL;
  } else if (stype=="double") {
    return DT_DOUBLE;
  } else if (stype=="int") {
    return DT_INT;
  } else if (stype=="string") {
    return DT_STRING;
  } else if (stype=="blob") {
    return DT_BLOB;
  } else if (stype=="external") {
    return DT_EXTERNAL;
  } else if (stype=="vector2d") {
    return DT_VECTOR2D;
  } else if (stype=="vector3d") {
    return DT_VECTOR3D;
  } else if (stype=="timeline") {
    return DT_TIMELINE;
  } else if (stype=="timevar") {
    return DT_TIMEVAR;
  } else if (stype=="list") {
    return DT_LIST;
  } else if (stype=="stringenum") {
    return DT_STRINGENUM;
  } else if (stype=="trigger") {
    return DT_TRIGGER;
  } else if (stype=="qwidget") {
    return DT_QWIDGET;
  } else {
    printf("Warning, undefined type: %s\n",stype.c_str());
    return DT_UNDEFINED;
  }
}

string VarData::typeToString(vDataTypeEnum vt)
{
  if (vt==DT_BOOL) {
    return "bool";
  } else if (vt==DT_DOUBLE) {
    return "double";
  } else if (vt==DT_INT) {
    return "int";
  } else if (vt==DT_STRING) {
    return "string";
  } else if (vt==DT_EXTERNAL) {
    return "external";
  } else if (vt==DT_BLOB) {
    return "blob";
  } else if (vt==DT_VECTOR2D) {
    return "vector2d";
  } else if (vt==DT_VECTOR3D) {
    return "vector3d";
  } else if (vt==DT_TIMEVAR) {
    return "timevar";
  } else if (vt==DT_TIMELINE) {
    return "timeline";
  } else if (vt==DT_LIST) {
    return "list";
  } else if (vt==DT_STRINGENUM) {
    return "stringenum";
  } else if (vt==DT_TRIGGER) {
    return "trigger";
  } else if (vt==DT_QWIDGET) {
    return "qwidget";
  } else {
    printf("warning: unknown vartype: %d\n",vt);
    return "undefined";
  }
}

VarData::VarData(string name)
{
  #ifndef VDATA_NO_THREAD_SAFETY
  //_mutex=PTHREAD_MUTEX_INITIALIZER;
  _mutex=new pthread_mutex_t;
  pthread_mutex_init((pthread_mutex_t*)_mutex, NULL);
  #endif
  DT_LOCK;
  _name=name;
  _flags=DT_FLAG_NONE;
  DT_UNLOCK;
  CHANGE_MACRO;

}

VarData::~VarData()
{
  #ifndef VDATA_NO_THREAD_SAFETY
  pthread_mutex_destroy((pthread_mutex_t*)_mutex);
  delete _mutex;
  #endif
}

VarDataFlag VarData::getRenderFlags() const {
  return _flags;
}

void VarData::setRenderFlags(VarDataFlag f) {
  DT_LOCK;
  VarDataFlag old=_flags;
  _flags=f;
  if (_flags != old) {
    DT_UNLOCK;
    CHANGE_MACRO;
  } else {
    DT_UNLOCK;
  }
}

void VarData::addRenderFlags(VarDataFlag f) {
  DT_LOCK;
  VarDataFlag old=_flags;
  _flags |= f;
  if (_flags != old) {
    DT_UNLOCK;
    CHANGE_MACRO;
  } else {
    DT_UNLOCK;
  }
}

void VarData::removeRenderFlags(VarDataFlag f) {
  DT_LOCK;
  VarDataFlag old=_flags;
  _flags = _flags & (~f);
  if (_flags != old) {
    DT_UNLOCK;
    CHANGE_MACRO;
  } else {
    DT_UNLOCK;
  }
}

bool VarData::areRenderFlagsSet(VarDataFlag f) const {
  DT_LOCK;
  bool answer=(_flags & f) == f;
  DT_UNLOCK;
  return (answer);
}

vector<VarData *> VarData::getChildren() const
{
  vector<VarData *> v;
  return v;
}

vDataTypeEnum VarData::getType() const
{
  return DT_UNDEFINED;
};

string VarData::getTypeName() const
{
  return VarData::typeToString(getType());
}

void VarData::setName(string name)
{
  DT_LOCK;
  if (name!=_name) {
    _name=name;
    DT_UNLOCK;
    CHANGE_MACRO;
  } else {
    DT_UNLOCK;
  }
}

string VarData::getName() const
{
  return _name;
}

void VarData::resetToDefault()
{
  CHANGE_MACRO;
}

void VarData::printdebug() const
{
}

//get a text-representation of this variable of this datatype
//note that this should be a human-readable string for visualization purposes
//it should not be a serialization of the data (getSerialString is meant for that
//purpose)
string VarData::getString() const
{
  return "";
}

bool VarData::setString(const string & val)
{
  (void)val;
  CHANGE_MACRO;
  return true;
}

//the following two functions are accessors for serialization
//if this is a binary type then base64 might be a good format for i/o
const char * VarData::getSerialString() const
{
  return getString().c_str();
}

void VarData::setSerialString(const string & val)
{
  setString(val);
}

//the following functions are especially useful for plotting:
bool VarData::hasValue() const
{
  return false;
}

bool VarData::hasMaxValue() const
{
  return false;
}

bool VarData::hasMinValue() const
{
  return false;
}

double VarData::getMinValue() const
{
  return 0.0;
}

double VarData::getMaxValue() const
{
  return 1.0;
}

double VarData::getValue() const
{
  return 0.0;
}


string VarData::fixString(const char * cst)
{
  if (cst==0) {
    return "";
  } else {
    return cst;
  }
}

string VarData::intToString(int val)
{
  char result[255];
  result[0]=0;
  sprintf(result,"%d",val);
  return result;
}

string VarData::doubleToString(double val)
{
  char result[255];
  result[0]=0;
  sprintf(result,"%lf",val);
  return result;
}

#ifndef VDATA_NO_XML
XMLNode VarData::findOrAppendChild(XMLNode & parent, string key, string val)
{
  //access ourselves:
  int n=parent.nChildNode("Var");
  int i,myIterator=0;
  //iterate through them and check if we already exist
  for (i=0; i<n; i++) {
    XMLNode t=parent.getChildNode("Var",&myIterator);
    if (fixString(t.getAttribute(key.c_str()))==val) {
      return t;
    }
  }
  return(parent.addChild("Var"));
}


void VarData::updateAttributes(XMLNode & us) const
{
  (void)us;
}

void VarData::updateText(XMLNode & us) const
{
  us.updateText(getSerialString());
}

void VarData::updateChildren(XMLNode & us) const
{
  //wipe out all children of type var...
  deleteAllVarChildren(us);
  vector<VarData *> v = getChildren();
  for (unsigned int i=0;i<v.size();i++) {
    //printf("writing: %d/%d (%s)\n",i,v.size(),v[i]->getName().c_str());
    v[i]->writeXML(us,true);
  }
}

void VarData::deleteAllVarChildren(XMLNode & node)
{
  int n=node.nChildNode("Var");
  int i=0;
  for (i=0; i<n; i++) {
    //fixed 08/06/2007: always delete first node in list, instead of using iterator
    XMLNode t=node.getChildNode("Var",0);
    t.deleteNodeContent(1);
  }
}

void VarData::writeXML(XMLNode & parent, bool blind_append) const
{
  if (areRenderFlagsSet(DT_FLAG_NOSAVE)) return;
  XMLNode us;
  if (blind_append) {
    //do a simple append to the XML parent
    us = parent.addChild("Var");
  } else {
    //do a name-based find and update, or append if it does not exist.
    us = findOrAppendChild(parent,"name",getName());
  }

  us.updateAttribute(getName().c_str(),"name","name");
  us.updateAttribute(getTypeName().c_str(),"type","type");

  updateAttributes(us);
  updateText(us);
  updateChildren(us);
}


void VarData::readAttributes(XMLNode & us)
{
  (void)us;
}

void VarData::readText(XMLNode & us)
{
  setSerialString(fixString(us.getText()));
}

void VarData::readChildren(XMLNode & us)
{
  (void)us;
}

void VarData::readXML(XMLNode & us)
{
  if (areRenderFlagsSet(DT_FLAG_NOLOAD)) return;
  readAttributes(us);
  readText(us);
  readChildren(us);
}

vector<VarData *> VarData::readChildrenHelper(XMLNode & parent , vector<VarData *> existing_children, bool only_update_existing, bool blind_append)
{
  //this again does non-destructive integration
  //As a result we update any predefined structures and
  //append any types that don't exist yet.

  vector<VarData *> result=existing_children;
  int n=parent.nChildNode("Var");
  int i,myIterator=0;

  //iterate through them and check if we already exist
  for (i=0; i<n; i++) {
    XMLNode t=parent.getChildNode("Var",&myIterator);
    string sname=fixString(t.getAttribute("name"));
    string stype=fixString(t.getAttribute("type"));
    vDataTypeEnum type=stringToType(stype);
    if (type!=DT_UNDEFINED) {
      bool found=false;
      if (sname!="" && blind_append==false) {
        //try to find existing child with this name
        for (unsigned j=0;j<existing_children.size();j++) {
          if (existing_children[j]->getName()==sname) {
            if (existing_children[j]->getType()==type) {
              existing_children[j]->readXML(t);
            } else {
              fprintf(stderr,"Type mismatch between XML and Data-Tree. Object name: %s. XML type was: %s. Internal type was: %s\n",sname.c_str(),stype.c_str(),existing_children[j]->getTypeName().c_str());
            }
            found=true;
            break;
            //ok matching name was found...update this node
          }
        }
      }
      if (found==false) {
        //printf("unable to find: %s in existing\n",sname.c_str());
        //the item did not already exist in the children...
        if (only_update_existing==false) {
          //create new node and append it:
          VarData * td=newVarType(type);
          td->setName(sname);
          td->readXML(t);
          result.push_back(td);
        }
      }
    } else {
      fprintf(stderr,"Found var with no or unknown type in XML... type: %s\n",stype.c_str());
    }
  }
  return result;
}
#endif
