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
  \file    VarTreeView.cpp
  \brief   C++ Implementation: VarTreeView
  \author  Stefan Zickler, (C) 2008
*/
#include "VarTreeView.h"
namespace VarTypes {
  VarTreeView::VarTreeView(VarTreeModel * tmodel) {
    model=tmodel;
    tw=new QTreeView();
    delegate = new VarItemDelegate();
    tw->setItemDelegate(delegate);
    tw->setAlternatingRowColors(true);
    tw->setUniformRowHeights ( false );
    tw->setAnimated(false);
    tw->setWordWrap(true);
    tw->setEditTriggers(tw->editTriggers() | QAbstractItemView::CurrentChanged);
    if (tmodel!=0) setModel(tmodel);
    search_edit = new QLineEdit(this);
    l = new QVBoxLayout();
    setLayout(l);
    l->addWidget(tw);
    l->addWidget(search_edit);
    l->setMargin(1);
    l->setSpacing(1);
    result_idx=0;
  
    connect(search_edit,SIGNAL(textChanged(const QString)),this,SLOT(search(const QString)));
    connect(search_edit,SIGNAL(returnPressed()),this,SLOT(nextSearchResult()));
  
  }
  
  void VarTreeView::search(const QString & text) {
    if (model==0) return;
    search_result=model->findItems(text,false);
    result_idx=0;
    if (search_result.size() > 0) {
      tw->scrollTo(search_result[result_idx]->index());
      tw->setCurrentIndex(search_result[result_idx]->index());
    }
  }
  
  void VarTreeView::nextSearchResult() {
    result_idx++;
    int n=search_result.size();
    if (result_idx > (n-1)) result_idx=0;
    if (n==0) return;
    tw->scrollTo(search_result[result_idx]->index());
    tw->setCurrentIndex(search_result[result_idx]->index());
  }
  
  VarTreeView::~VarTreeView() {
    delete delegate;
    delete tw;
    delete l;
  }
  
  void VarTreeView::setModel(VarTreeModel * tmodel) {
    if (model!=0) {
      //remove any old signals from the model to this view
      disconnect(model,0,this,0);
    }
  
  
    search_result.clear();
    result_idx=0;
  
    model=tmodel;
    tw->setModel(model);
    fitColumns();
  
    //open persistent editors for any items which to not have one yet:
  
    if (model!=0) {
      //connect the model to open persistent editors for new items:
      //connect(model,SIGNAL(rowsInserted(const QModelIndex &, int, int)),this,SLOT(newItemChecksRows(const QModelIndex &, int, int)));
      connect(model,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),this,SLOT(checkDataChanged(const QModelIndex &, const QModelIndex &)));
    }
  
  }
  
  
  void VarTreeView::checkDataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight ) {
    for (int i=topLeft.row();i<=bottomRight.row();i++) {
      if (topLeft.column()==1) {
        QModelIndex index=topLeft.sibling(i,1);
        if (index.isValid() && index.model()!=0) {
          VarItem * item=(VarItem*)(((VarTreeModel*)index.model())->itemFromIndex (index));
          if (item!=0) {
            VarType * dt=item->getVarType();
            if (dt!=0) {
              if ((dt->getFlags() & VARTYPE_FLAG_PERSISTENT) != 0x00) {
                tw->openPersistentEditor(index);
              } /*else {
                tw->closePersistentEditor(index);
              }*/
            }
          }
        }
      }
    }
  
  }
  void VarTreeView::newItemChecksRows(const QModelIndex & parent, int start, int end) {
    (void)parent;
    (void)start;
    (void)end;
    /*  
    printf("parent: %d  %d new row: %d to %d\n",parent.row(),parent.column(),start,end); fflush(stdout);
    if (parent.row() != -1 && parent.column() != -1) {
      
      //tw->openPersistentEditor(parent.child(start,0));
    }*/
  }
  
  /*void VarTreeView::newItemChecks(const QModelIndex & index) {
    //FIXME: check if item has a persistent editor...
    //       if so, create it!
    
    
  }*/
  
  void VarTreeView::fitColumns() {
    tw->resizeColumnToContents(0);
    tw->resizeColumnToContents(1);
  }
  
  void VarTreeView::expandAndFocus(VarType * search) {
    if (model==0) return;
    QList<VarItem *> res=model->findItems(search);
    if (res.size() > 0) {
      tw->expand(res[0]->index());
      tw->scrollTo(res[0]->index());
      tw->setCurrentIndex(res[0]->index());
    }
  }
};
