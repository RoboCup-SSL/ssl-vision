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
  \file    VarList.h
  \brief   C++ Interface: VarList
  \author  Stefan Zickler, (C) 2008
*/

#ifndef DATAGROUP_H_
#define DATAGROUP_H_
#include "primitives/VarData.h"
#include <vector>
using namespace std;


/*!
  \class  VarList
  \brief  This is the list type of the VarTypes system
  \author Stefan Zickler, (C) 2008
  \see    VarTypes.h

  VarList allows the storage of an ordered list of children nodes.

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/

class VarList : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:
  vector<VarData *> list;
public:
  /// Creates an empty VarList
  VarList(string _name="");
  virtual ~VarList();

  /// this will always return the empty string ""
  virtual string getString() const { return ""; }

  /// this will clear the list
  virtual void resetToDefault() { list.clear(); };

  /// prints the label and number of elements
  virtual void printdebug() const { printf("VarList named %s containing %d element(s)\n",getName().c_str(), list.size()); }

  /// adds a VarData item to the end of the list.
  int addChild(VarData * child) { DT_LOCK; list.push_back(child); DT_UNLOCK; CHANGE_MACRO; return (list.size()-1);}
  bool removeChild(VarData * child) {
    vector<VarData *> newlist;
    unsigned int n=list.size();
    bool found=false;
    for (unsigned int i=0;i<n;i++) {
      if (list[i]!=child) {
        newlist.push_back(list[i]);
      } else {
        found=true;
      }
    }
    if (found) {
      list=newlist;
      CHANGE_MACRO;
    }
    return found;
  };

  virtual vDataTypeEnum getType() const { return DT_LIST; } ;

  /// returns a vector of all children in the order that they occur in internally
  virtual vector<VarData *> getChildren() const
  {
    DT_LOCK;
    vector<VarData *> l = list;
    DT_UNLOCK;
    return l;
  }

#ifndef VDATA_NO_XML
protected:
  virtual void readChildren(XMLNode & us)
  {
    list=readChildrenHelper(us, list, false, false);
    CHANGE_MACRO;
  }
#endif
};

#endif /*DATAGROUP_H_*/
