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
  \file    VarType.cpp
  \brief   C++ Implementation: VarType
  \author  Stefan Zickler, (C) 2008
*/

#include "VarType.h"
#include "../gui/VarItemDelegate.h"
#include "VarTypesInstance.h"
namespace VarTypes {
  
  VarType::VarType(string name)
  {
    #ifndef VDATA_NO_THREAD_SAFETY
    _mutex=new QMutex();
    #endif
    lock();
    _name=name;
    _flags=VARTYPE_FLAG_NONE;
    unlock();
    changed();
  
  }
  
  VarType::~VarType()
  {
    #ifndef VDATA_NO_THREAD_SAFETY
    delete _mutex;
    #endif
  }
  
  VarTypeFlag VarType::getFlags() const {
    return _flags;
  }
  
  void VarType::setFlags(VarTypeFlag f) {
    lock();
    VarTypeFlag old=_flags;
    _flags=f;
    if (_flags != old) {
      unlock();
      changed();
    } else {
      unlock();
    }
  }
  
  void VarType::addFlags(VarTypeFlag f) {
    lock();
    VarTypeFlag old=_flags;
    _flags |= f;
    if (_flags != old) {
      unlock();
      changed();
    } else {
      unlock();
    }
  }
  
  void VarType::removeFlags(VarTypeFlag f) {
    lock();
    VarTypeFlag old=_flags;
    _flags = _flags & (~f);
    if (_flags != old) {
      unlock();
      changed();
    } else {
      unlock();
    }
  }
  
  bool VarType::areFlagsSet(VarTypeFlag f) const {
    lock();
    bool answer=(_flags & f) == f;
    unlock();
    return (answer);
  }
  
  vector<VarType *> VarType::getChildren() const
  {
    vector<VarType *> v;
    return v;
  }
  
  void VarType::setName(string name)
  {
    lock();
    if (name!=_name) {
      _name=name;
      unlock();
      changed();
    } else {
      unlock();
    }
  }
  
  string VarType::getName() const
  {
    return _name;
  }
  
  void VarType::resetToDefault()
  {
    changed();
  }
  
  string VarType::fixString(const char * cst)
  {
    if (cst==0) {
      return "";
    } else {
      return cst;
    }
  }
  
  string VarType::intToString(int val)
  {
    char result[255];
    result[0]=0;
    sprintf(result,"%d",val);
    return result;
  }
  
  string VarType::doubleToString(double val)
  {
    char result[255];
    result[0]=0;
    sprintf(result,"%lf",val);
    return result;
  }
  
  #ifndef VDATA_NO_XML
  XMLNode VarType::findOrAppendChild(XMLNode & parent, string key, string val)
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
  
  
  void VarType::updateAttributes(XMLNode & us) const
  {
    (void)us;
  }
  
  void VarType::updateText(XMLNode & us) const
  {
    string tmp;
    getSerialString(tmp);
    us.updateText(tmp.c_str());
  }
  
  void VarType::updateChildren(XMLNode & us) const
  {
    //wipe out all children of type var...
    deleteAllVarChildren(us);
    vector<VarType *> v = getChildren();
    for (unsigned int i=0;i<v.size();i++) {
      //printf("writing: %d/%d (%s)\n",i,v.size(),v[i]->getName().c_str());
      v[i]->writeXML(us,true);
    }
  }
  
  void VarType::deleteAllVarChildren(XMLNode & node)
  {
    int n=node.nChildNode("Var");
    int i=0;
    for (i=0; i<n; i++) {
      //fixed 08/06/2007: always delete first node in list, instead of using iterator
      XMLNode t=node.getChildNode("Var",0);
      t.deleteNodeContent(1);
    }
  }
  
  void VarType::writeXML(XMLNode & parent, bool blind_append)
  {
    if (areFlagsSet(VARTYPE_FLAG_NOSAVE)) return;
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
    emit(XMLwasWritten(this));
  }
  
  
  void VarType::readAttributes(XMLNode & us)
  {
    (void)us;
  }
  
  void VarType::readText(XMLNode & us)
  {
    setSerialString(fixString(us.getText()));
  }
  
  void VarType::readChildren(XMLNode & us)
  {
    (void)us;
  }
  
  void VarType::readXML(XMLNode & us)
  {
    if (areFlagsSet(VARTYPE_FLAG_NOLOAD)) return;
    //printf("readXML: %s\n",this->getName().c_str());
    if (areFlagsSet(VARTYPE_FLAG_NOLOAD_ATTRIBUTES)==false) readAttributes(us);
    readText(us);
    readChildren(us);
    emit(XMLwasRead(this));
  }
  
  void VarType::loadExternal() {
  
  }
  
  vector<VarType *> VarType::readChildrenHelper(XMLNode & parent , vector<VarType *> existing_children, bool only_update_existing, bool blind_append)
  {
    //this again does non-destructive integration
    //As a result we update any predefined structures and
    //append any types that don't exist yet.
    vector<VarType *> result=existing_children;
    int n=parent.nChildNode("Var");
    int i,myIterator=0;
  
    //iterate through them and check if we already exist
    set<VarType *> unmatched_children;
    for (unsigned int j=0;j<existing_children.size();j++) {
      unmatched_children.insert(existing_children[j]);
    }
    for (i=0; i<n; i++) {
      XMLNode t=parent.getChildNode("Var",&myIterator);
      string sname=fixString(t.getAttribute("name"));
      string stype=fixString(t.getAttribute("type"));
      VarTypeId type=VarTypesInstance::getFactory()->stringToType(stype);
      if (type!=VARTYPE_ID_UNDEFINED) {
        bool found=false;
        if (sname!="" && blind_append==false) {
          //try to find existing child with this name
          for (unsigned int j=0;j<existing_children.size();j++) {
            if (existing_children[j]->getName()==sname) {
              if (existing_children[j]->getType()==type) {
                existing_children[j]->readXML(t);
              } else {
                fprintf(stderr,"Type mismatch between XML and Data-Tree. Object name: %s. XML type was: %s. Internal type was: %s\n",sname.c_str(),stype.c_str(),existing_children[j]->getTypeName().c_str());
              }
              unmatched_children.erase(existing_children[j]);
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
            VarType * td= VarTypesInstance::getFactory()->newVarType(type);
            td->setName(sname);
            td->readXML(t);
            result.push_back(td);
          }
  
        }
      } else {
        fprintf(stderr,"Found var with no or unknown type in XML... type: %s\n",stype.c_str());
      }
    }
    //---fix for recursing tree that's not defined in xml yet:
    queue<VarType *> _queue;
    for (set<VarType *>::iterator iter = unmatched_children.begin(); iter!=unmatched_children.end(); iter++) {
      if (*iter!=0) _queue.push(*iter);
    }
    //recurse all unmatched children to make sure we load all externals:
    while(_queue.empty()==false) {
      VarType * d = _queue.front();
      _queue.pop();
      d->loadExternal();
      vector<VarType *> children = d->getChildren();
      int s=children.size();
      for (int i=0;i<s;i++) {
        if (children[i]!=0) _queue.push(children[i]);
      }
    }
    //---end of fix
    return result;
  }
  
  // Finds a child based on its label
  // Returns 0 if not found.
  VarType * VarType::findChild(string label) const {
    vector<VarType *> children = getChildren();
    unsigned int s = children.size();
    for (unsigned int i=0;i<s;i++) {
      if (children[i]->getName().compare(label)==0) return (children[i]);
    }
    return 0;
  }
  
  
  //------------MODEL VIEW GUI STUFF:
  void VarType::paint (const VarItemDelegate * delegate, QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
    //let the QT default delegate do the painting:
    delegate->QItemDelegate::paint(painter,option,index);
  }
  
  QWidget * VarType::createEditor(const VarItemDelegate * delegate, QWidget *parent, const QStyleOptionViewItem &option) {
    (void)delegate;
    (void)option;
    return new QLineEdit(parent);
  }
  
  void VarType::setEditorData(const VarItemDelegate * delegate, QWidget *editor) const {
    (void)delegate;
    QLineEdit * ledit=(QLineEdit *) editor;
    ledit->setText(QString::fromStdString(getString()));
  }
  
  void VarType::setModelData(const VarItemDelegate * delegate, QWidget *editor) {
    (void)delegate;
    QLineEdit * ledit=(QLineEdit *) editor;
    if (setString(ledit->text().toStdString())) mvcEditCompleted();
  }
  
  QSize VarType::sizeHint(const VarItemDelegate * delegate, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    return (delegate->QItemDelegate::sizeHint(option, index));
  }
};
#endif
