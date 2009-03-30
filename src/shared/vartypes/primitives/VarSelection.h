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
  \file    VarSelection.h
  \brief   C++ Interface: VarSelection
  \author  Stefan Zickler, (C) 2008
*/

#ifndef VAR_SELECTION_H_
#define VAR_SELECTION_H_
#include "primitives/VarData.h"
#include "primitives/VarBool.h"
#include <vector>
#include <QListWidget>
using namespace std;

/*!
  \class  VarSelection
  \brief  This is the multi-selection VarType of the VarTypes system
  \author Stefan Zickler, (C) 2008
  \see    VarTypes.h

  The VarSelection class allows to define an ordered set of strings from which the user can select an arbitrary combination. This effectively models a popup / listbox allowing multi-item selection.

  If you don't know what VarTypes are, please see \c VarTypes.h 
*/
class VarSelection : public VarData
{
#ifndef VDATA_NO_QT
  Q_OBJECT
#endif
protected:
  vector<VarData *> list;
public:
  VarSelection(string _name="", int num_items=0, bool default_value=false);
  virtual ~VarSelection();

  virtual string getString() const { 
    string result;
    DT_LOCK;
    //TODO:
    result="";
    //for (unsigned int i =0; i < list.size(); i++) {
    //  list[i]->getName()
    //}
    DT_UNLOCK;
    return result;
  }
  virtual void printdebug() const { printf("VarSelection named %s containing %d element(s)\n",getName().c_str(), list.size()); }

  unsigned int getCount() const {
    DT_LOCK;
    int n = list.size();
    DT_UNLOCK;
    return n;
  }

  /// check whether an item is currently selected
  bool isSelected(unsigned int i) const {
    bool result;
    DT_LOCK;
    if (i >= list.size()) {
      result=false;
    } else {
      result=((VarBool *)list[i])->getBool();
    }
    DT_UNLOCK;
    return result;
  }

  /// select an item by using its index
  bool setSelected(unsigned int i, bool val) {
    bool result=false;
    DT_LOCK;
    if (i < list.size()) {
      result=(((VarBool *)list[i])->setBool(val));
    }
    DT_UNLOCK;
    if (result) CHANGE_MACRO;
    return result;
  }

  /// get the string of item at a given index
  string getLabel(unsigned int index) const {
   string result="";
   DT_LOCK;
   if (index >= list.size()) {
     result="";
   } else {
     result=((VarBool *)list[index])->getName();
   }
   DT_UNLOCK;
   return result;
  }

  /// trim or extend the list to a certain total number of items
  void setSize(unsigned int size, bool default_val=false) {
    DT_LOCK;
    if (list.size() < size) {
      for (unsigned int i=list.size();i<size;i++) {
        char tmp[64];
        sprintf(tmp,"%d",list.size());
        list.push_back(new VarBool(tmp ,default_val));
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
    return;
  }

  /// set the string of item at a given index
  void setLabel(unsigned int index, const string & label) {
   string result="";
   DT_LOCK;
   if (index < list.size()) {
     list[index]->setName(label);
   }
   DT_UNLOCK;
   CHANGE_MACRO;
   return;
  }

  /// add an item to the end of the enumeration
  int addItem(bool value=false, string label="") {
    DT_LOCK;
    if (label.compare("")==0) {
      char tmp[64];
      sprintf(tmp,"%d",list.size());
      list.push_back(new VarBool(tmp , value));
    } else {
      list.push_back(new VarBool(label , value));
    }
    list[list.size()-1]->addRenderFlags(DT_FLAG_HIDDEN);
    DT_UNLOCK;
    CHANGE_MACRO;
    return (list.size()-1);
  }

  virtual vDataTypeEnum getType() const { return DT_SELECTION; } ;

  virtual vector<VarData *> getChildren() const
  {
    DT_LOCK;
    vector<VarData *> l;
    unsigned int n_children=list.size();
    l.reserve(n_children);
    for (unsigned int i=0; i < n_children; i++) { 
      l.push_back(list[i]);
    }
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
    (void)option;
    QListWidget * w=new QListWidget(parent);
    connect((const QObject *)w,SIGNAL(itemChanged( QListWidgetItem * )),(const QObject *)delegate,SLOT(editorChangeEvent()));
    w->setSelectionMode(QAbstractItemView::SingleSelection);
    return w;
  }
  virtual void setEditorData(const VarItemDelegate * delegate, QWidget *editor) const {
    (void)delegate;
    QListWidget * listwidget=(QListWidget*)editor;
    int n = getCount();
    QString tmp;
    listwidget->clear();
    for (int i=0;i<n;i++) {
      tmp=QString::fromStdString(getLabel(i));
      QListWidgetItem * item = new QListWidgetItem(tmp);
      item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
      item->setCheckState(isSelected(i) ? Qt::Checked : Qt::Unchecked);
      //item->setSelected();
      listwidget->addItem(item);
    }

  }

  virtual void setModelData(const VarItemDelegate * delegate, QWidget *editor) {
    (void)delegate;
    QListWidget * listwidget=(QListWidget *)editor;
    bool changed=false;
    int in=listwidget->count();
    for (int i=0;i<in;i++) {
      QListWidgetItem * item = listwidget->item(i);
      if (item!=0) {
        changed = changed | (setSelected(i,item->checkState()==Qt::Checked));
      }
    }
    if (changed) mvcEditCompleted();
  }

  virtual QSize sizeHint(const VarItemDelegate * delegate, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    (void)delegate;
    (void)option;
    (void)index;
    QSize s(20,100);
    return s;
  }

};

#endif
