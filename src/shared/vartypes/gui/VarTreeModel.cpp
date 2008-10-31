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
  \file    VarTreeModel.h
  \brief   C++ Implementation: VarTreeModel
  \author  Stefan Zickler, (C) 2008
*/
#include "VarTreeModel.h"

VarTreeModel::VarTreeModel() : QStandardItemModel()
{
  resetViewOptions();
  updateHeader();
}

VarTreeModel::~VarTreeModel()
{}

void VarTreeModel::resetViewOptions() {
  vector<VarColumnFlag> v;
  v.push_back(DT_COL_TREE_NODE|DT_COL_ICON|DT_COL_TEXT_LABEL);
  v.push_back(DT_COL_TEXT_VALUE|DT_COL_EDITABLE|DT_COL_RANGEBARS);
  opts.setColumns(v);
}

const VarTreeViewOptions * VarTreeModel::getViewOptions() const {
  return &opts;
}

void VarTreeModel::updateHeader() {
  vector<VarColumnFlag> v=opts.getColumns();
  for (unsigned int i = 0;i<v.size();i++) {
    QStandardItem * head=new QStandardItem();
    if ((v[i] & (DT_COL_TEXT_LABEL)) != 0) {
      head->setText("Variable");
    } else if ((v[i] & (DT_COL_TEXT_VALUE)) != 0) {
      head->setText("Value");
    } else {
      head->setText("");
    }
    head->setTextAlignment(Qt::AlignLeft);
    setHorizontalHeaderItem(i,head);
  }
}

QList<VarItem *> VarTreeModel::findItems ( const QString & text, bool case_sensitive ) const {
  QList<VarItem *> result;
  QList<QStandardItem *> tmp;
  tmp = QStandardItemModel::findItems(text,Qt::MatchRecursive|(case_sensitive ? (Qt::MatchContains|Qt::MatchCaseSensitive) : Qt::MatchContains),0);
  QList<QStandardItem *>::const_iterator iter = tmp.constBegin();
  QList<QStandardItem *>::const_iterator end = tmp.constEnd();
  while(iter != end) {
    result.append((VarItem *)(*iter));
    iter++;
  }
  return result;
}

QList<VarItem *> VarTreeModel::findItems(const VarData * item) const {
  QList<VarItem *> result;
  if (item!=0) VarItem::searchTree(invisibleRootItem(), item, result);
  return result;
}

void VarTreeModel::setRootItems(vector<VarData *> items) {
  VarItem::updateTree(invisibleRootItem(),items,getViewOptions(),true);
}

void VarTreeModel::setRootItem(VarData * item) {
  vector<VarData *> v;
  v.push_back(item);
  setRootItems(v); 
}

