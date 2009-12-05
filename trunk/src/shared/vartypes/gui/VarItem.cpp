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
  \file    VarItem.cpp
  \brief   C++ Implementation: VarItem
  \author  Stefan Zickler, (C) 2008
*/
#include "VarItem.h"

namespace VarTypes {
  
  VarItem::VarItem(VarType * _dt, const VarTreeViewOptions * opts, GuiColumnFlag myflags) {
    dt=0;
    update(_dt, opts, myflags);
  }
  
  VarItem::~VarItem() {
  
  }
  
  void VarItem::changeUpdate() {
    update(dt,opts,colflags);
    vector<VarType *> vd;
    if (dt!=0) {
      //update tree structure if it's a list item.
      if (areColFlagsSet(colflags,GUI_COLUMN_FLAG_TREE_NODE)) {
        if (dt->getType()==VARTYPE_ID_LIST || dt->getType()==VARTYPE_ID_EXTERNAL) {
          if (dt->areFlagsSet(VARTYPE_FLAG_HIDE_CHILDREN)==false) {
            vd=((VarList*)dt)->getChildren();
          }
          updateTree(this,vd,opts,true);
        }
      }
    }
  }
  
  int VarItem::type() const {
    return QStandardItem::UserType + 1;
  }
  void VarItem::update(VarType * _dt, const VarTreeViewOptions * _opts, GuiColumnFlag myflags) {
    opts=_opts;
    colflags=myflags;
    setEditable(areColFlagsSet( colflags,GUI_COLUMN_FLAG_EDITABLE) && (_dt->getType() != VARTYPE_ID_LIST && _dt->getType() != VARTYPE_ID_EXTERNAL ));
    if (_dt!=dt && _dt!=0) {
      if (dt != 0) {
        disconnect(dt,SIGNAL(hasChanged(VarType *)),this,SLOT(changeUpdate()));
      }
      connect(_dt,SIGNAL(hasChanged(VarType *)),this,SLOT(changeUpdate()));
      dt=_dt;
    }
    if (dt!=0) {
    setEnabled(! dt->areFlagsSet(VARTYPE_FLAG_READONLY));
      if (areColFlagsSet(colflags,GUI_COLUMN_FLAG_TREE_NODE)) {
        setText(QString::fromStdString(dt->getName()));
        setToolTip(QString::fromStdString(dt->getName()) + " (" + QString::fromStdString(dt->getTypeName()) + ")");
        if (dt->getType() == VARTYPE_ID_LIST) {
          setIcon(QIcon(":/icons/vartypes/list.png"));
        } else if (dt->getType() == VARTYPE_ID_EXTERNAL) {
          setIcon(QIcon(":/icons/vartypes/external.png"));
        } else if (dt->getType() == VARTYPE_ID_TIMELINE) {
          setIcon(QIcon(":/icons/vartypes/time.png"));
          //setCheckState(1,Qt::Unchecked);
        } else {
          setIcon(QIcon(":/icons/vartypes/node.png"));
        }
      } else if (areColFlagsSet(colflags,GUI_COLUMN_FLAG_TEXT_VALUE)) {
        if ((dt->getFlags() & VARTYPE_FLAG_PERSISTENT) != 0x00) {
          setText("");
        } else {
          setText(QString::fromStdString(dt->getString()));
        }
      }
    }
  }
  
  void VarItem::searchTree(QStandardItem * node, const VarType * search, QList<VarItem *> & result) {
    int n=node->rowCount();
    QStandardItem * tmp;
    for (int i=0;i<n;i++) {
      tmp=node->child(i);
      if (((VarItem *)tmp)->getVarType()==search) result.append((VarItem *)tmp);
      searchTree(tmp,search,result);
    }
  }
  
  void VarItem::updateTree(QStandardItem * node, const vector<VarType *> & children, const VarTreeViewOptions * _opts, bool recurse) {
    //FIRST OF ALL filter out child_list that are invisible to the user or that are null-pointers
    vector<VarType *> child_list;
    for (unsigned int i=0;i<children.size();i++) {
      VarType * d = children[i];
      if (d!=0 && d->areFlagsSet(VARTYPE_FLAG_HIDDEN)==false) {
        child_list.push_back(d);
      }
    }
  
    vector<GuiColumnFlag> columns=_opts->getColumns();
    //Now display visible child_list:
    node->setColumnCount(columns.size());
    node->setRowCount(child_list.size());
    
    for (unsigned int i=0;i<child_list.size();i++) {
      for (unsigned int j=0;j<columns.size();j++) {
        VarItem * item=(VarItem *)node->child(i,j);
        //printf("i: %d   j: %d\n",i,j);
        if (item==0) {
          item=new VarItem(child_list[i],_opts,columns[j]);
          node->setChild(i,j,item);
        } else {
          item->update(child_list[i],_opts,columns[j]);
        }
  
        if (recurse && j==0) {
          if (child_list[i]->areFlagsSet(VARTYPE_FLAG_HIDE_CHILDREN)==false) {
            updateTree(item,child_list[i]->getChildren(),_opts,recurse);
          }
        }
      }
    }
  }
  
  VarType * VarItem::getVarType() {
    return dt;
  }
  
  GuiColumnFlag VarItem::getColFlags() const {
    return colflags;
  }
  
  const VarTreeViewOptions * VarItem::getViewOptions() const {
    return opts;
  }
};
