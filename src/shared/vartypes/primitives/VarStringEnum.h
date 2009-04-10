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
  \file    VarStringEnum.h
  \brief   C++ Interface: VarStringEnum
  \author  Stefan Zickler, (C) 2008
*/

#ifndef STRINGENUM_H_
#define STRINGENUM_H_
#include "primitives/VarData.h"
#include "primitives/VarString.h"
#include <vector>
#include <QComboBox>
using namespace std;

/*!
  \class  VarStringEnum
  \brief  This is the string enumeration VarType of the VarTypes system
  \author Stefan Zickler, (C) 2008
  \see    VarTypes.h

  The VarStringEnum class allows to define an ordered set of strings from which the user can chose one. This effectively models a popup / listbox allowing
  a single-item selection.

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/
class VarStringEnum : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:
  vector<VarData *> list;
  string selected;
  string default_string;
public:
  VarStringEnum(string _name="", string _default="");
  virtual ~VarStringEnum();

  virtual string getString() const { return selected; }
  virtual void resetToDefault() { list.clear(); selected=default_string; };
  virtual void printdebug() const { printf("StringEnum named %s containing %d element(s)\n",getName().c_str(), list.size()); }

  unsigned int getCount() const {
    DT_LOCK;
    int n = list.size();
    DT_UNLOCK;
    return n;
  }

  /// set the currently selected string
  virtual bool setString(const string & val) {
    return select(val);
  }

  /// get the index of the currently selected string item
  int getIndex() const {
    DT_LOCK;
    int res=-1;
    unsigned int n = list.size();
    for (unsigned int i=0;i<n;i++) {
      if (((VarString *)(list[i]))->getString().compare(selected)==0) {
        res= i;
        break;
      }
    }
    DT_UNLOCK;
    return res;
  }

  /// select an item by using its index
  bool selectIndex(unsigned int i) {
    string result="";
    DT_LOCK;
    if (i > (list.size() - 1)) {
      result=default_string;
    } else {
      result=((VarString *)(list[i]))->getString();
    }
    if (result!=selected) {
      selected=result;
      DT_UNLOCK; CHANGE_MACRO; return true;
    } else {
      DT_UNLOCK; return false;
    }
  }

  /// select a particular string
  bool select(const string & val) {
     //TODO: add a flag so only selection of existing items is possible.
     DT_LOCK; if (val!=selected) {selected=val; DT_UNLOCK; CHANGE_MACRO; return true;} else { DT_UNLOCK; return false;}
  }

  /// return the currently selected string
  string getSelection() const {
    return selected;
  }

  /// get the string of item at a given index
  string getLabel(unsigned int index) const {
   string result="";
   DT_LOCK;
   if (index > (list.size() - 1)) {
     result="";
   } else {
     result=((VarString *)(list[index]))->getString();
   }
   DT_UNLOCK;
   return result;
  }

  /// trim or extend the list to a certain total number of items
  void setSize(unsigned int size, const string default_label="") {
    DT_LOCK;
    if (list.size() < size) {
      for (unsigned int i=list.size();i<size;i++) {
        char tmp[64];
        sprintf(tmp,"%d",list.size());
        list.push_back(new VarString(tmp ,default_label));
        list[list.size()-1]->addRenderFlags(DT_FLAG_HIDDEN);
      }
    } else if (list.size() > size) {
      for (unsigned int i=list.size();i>size;i--) {
        delete list[i-1];
        list.pop_back();
      }
    }
    DT_UNLOCK;
    CHANGE_MACRO;
  }

  /// set the string of item at a given index
  void setLabel(unsigned int index, const string & label) {
   string result="";
   DT_LOCK;
   if (index < list.size()) {
     if (((VarString *)(list[index]))->getString().compare(selected)==0) {
       selected=label;
     }
     ((VarString *)(list[index]))->setString(label);
     
   }
   
   DT_UNLOCK;
   CHANGE_MACRO;
   return;
  }

  /// add an item to the end of the enumeration
  int addItem(const string & label) {
    DT_LOCK;
    char tmp[64];
    sprintf(tmp,"%d",list.size());
    list.push_back(new VarString(tmp ,label));
    list[list.size()-1]->addRenderFlags(DT_FLAG_HIDDEN);
    DT_UNLOCK;
    CHANGE_MACRO;
    return (list.size()-1);
  }

  virtual vDataTypeEnum getType() const { return DT_STRINGENUM; } ;

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
    if (areRenderFlagsSet(DT_FLAG_NOLOAD_ENUM_CHILDREN)==false) {
      list=readChildrenHelper(us, list, false, false);
      for (unsigned int i = 0; i < list.size();i++) {
        list[i]->addRenderFlags(DT_FLAG_HIDDEN);
      }
    }
    CHANGE_MACRO;
  }
#endif

//Qt model/view gui stuff:
public:
  virtual QWidget * createEditor(const VarItemDelegate * delegate, QWidget *parent, const QStyleOptionViewItem &option) {
    //TODO add connect...
    (void)delegate;
    (void)parent;
    (void)option;
    QComboBox * w = new QComboBox(parent);
    connect((const QObject *)w,SIGNAL(currentIndexChanged( int )),(const QObject *)delegate,SLOT(editorChangeEvent()));
    return w;
  }
  
  virtual void setEditorData(const VarItemDelegate * delegate, QWidget *editor) const {
    (void)delegate;
    QComboBox * combo =(QComboBox *)editor;
    int n = getCount();
    QString tmp;
    combo->clear();
    for (int i=0;i<n;i++) {
      tmp=QString::fromStdString(getLabel(i));
      combo->insertItem(combo->count(), tmp);
      if (tmp==QString::fromStdString(getString())) {
        combo->setCurrentIndex(i);
      }
    }
  }

  virtual void setModelData(const VarItemDelegate * delegate, QWidget *editor) {
    (void)delegate;
    QComboBox * combo=(QComboBox *) editor;
    if (select(combo->currentText().toStdString())) mvcEditCompleted();
  };

};

#endif /*DATAGROUP_H_*/