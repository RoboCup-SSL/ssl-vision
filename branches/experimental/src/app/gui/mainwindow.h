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
  \file    mainwindow.h
  \brief   C++ Interface: MainWindow
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "affinity_manager.h"
#include <QtGui>
#include <qmainwindow.h>
#include "ui_mainwindow.h"
#include <QSplitter>
#include "capture_thread.h"
#include "videowidget.h"
#include "glwidget.h"
#include "gui/VarTreeModel.h"
#include "gui/VarItem.h"
#include "gui/VarTreeView.h"
#include "glLUTwidget.h"
#include "lutwidget.h"
#include <QStringList>
#include "VarXML.h"
#include "stacks.h"
#include "qgetopt.h"
#include "multistacks.h"
/*!
  \class   MainWindow
  \brief   The ssl-vision main window
  \author  Stefan Zickler, (C) 2008
*/
class MainWindow : public QMainWindow, public Ui_MainWindow
{
    Q_OBJECT

public:
  AffinityManager * affinity;
  //GetOpt * opt;
  VarList * root;
  VarTreeView * tree_view;
  QTabWidget * left_tab;
  QTabWidget * right_tab;
  QSplitter * splitter;
  QSplitter * splitter2;
  VarList * settings;
  vector<VarType * > world;
  VarTreeModel * tmodel;
  vector<RealTimeDisplayWidget *> display_widgets;
  vector<QSplitter *> stack_widgets;
  RenderOptions * opts;

  MultiVisionStack * multi_stack;

  MainWindow(bool start_capture, bool enforce_affinity);
  virtual ~MainWindow();
  void init();

  virtual void closeEvent(QCloseEvent * event );
  virtual void timerEvent(QTimerEvent * e);
};


#endif
