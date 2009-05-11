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
signals:
  void childAdded(VarData * child);
  void childRemoved(VarData * child);
public:
  /// Creates an empty VarList
  VarList(string _name="");
  virtual ~VarList();

  /// this will always return the empty string ""
  virtual string getString() const { return ""; }

  /// this will clear the list
  virtual void resetToDefault() {DT_LOCK; for (unsigned int i=0;i<list.size();i++) { emit(childRemoved(list[i])); } list.clear();  DT_UNLOCK; };

  /// prints the label and number of elements
  virtual void printdebug() const { printf("VarList named %s containing %zu element(s)\n",getName().c_str(), list.size()); }

  /// adds a VarData item to the end of the list.
  int addChild(VarData * child) { DT_LOCK; list.push_back(child); emit(childAdded(child)); DT_UNLOCK; CHANGE_MACRO; return (list.size()-1);}
  bool removeChild(VarData * child) {
    DT_LOCK;
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
      emit(childRemoved(child));
      list=newlist;
      CHANGE_MACRO;
    }
    DT_UNLOCK;
    return found;
  };

  virtual vDataTypeEnum getType() const { return DT_LIST; } ;

  /// returns the number of children of this list
  virtual int getChildrenCount() const
  {
    int res;
    DT_LOCK;
    res = list.size();
    DT_UNLOCK;
    return res;
  }

  /// returns a vector of all children in the order that they occur in internally
  virtual vector<VarData *> getChildren() const
  {
    DT_LOCK;
    vector<VarData *> l = list;
    DT_UNLOCK;
    return l;
  }

  /// Finds a child based on the label of 'other'
  /// If the child is not found then other is returned
  /// However, if the child *is* found then other will be *deleted* and the child will be returned!
  VarData * findChildOrReplace(VarData * other) {
    VarData * data = findChild(other->getName());
    if (data!=0) {
      delete other;
      return data;
    } else {
      addChild(other);
      return other;
    }
  }

  /// Finds a child based on the label of 'other'
  /// If the child is not found then other is returned
  /// However, if the child *is* found then other will be *deleted* and the child will be returned!
  template <class VCLASS> 
  VCLASS * findChildOrReplace(VCLASS * other) {
    VCLASS * data = (VCLASS *)findChild(other->getName());
    if (data!=0) {
      delete other;
      return data;
    } else {
      addChild(other);
      return other;
    }
  }


#ifndef VDATA_NO_XML
protected:
  virtual void readChildren(XMLNode & us)
  {
    DT_LOCK;
    int before=list.size();
    list=readChildrenHelper(us, list, false, false);
    int after=list.size();
    if (after > before) {
      for (int i=before; i < after; i++) {
         emit(childAdded(list[i]));
      }
    }
    DT_UNLOCK;
    CHANGE_MACRO;
  }
#endif

//Qt model/view gui stuff:
public:
virtual QWidget * createEditor(const VarItemDelegate * delegate, QWidget *parent, const QStyleOptionViewItem &option) {
  (void)delegate;
  (void)option;
  (void)parent;
  return 0;
}


};


#endif /*DATAGROUP_H_*/
