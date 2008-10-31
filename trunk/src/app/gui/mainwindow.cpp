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
  \file    mainwindow.cpp
  \brief   C++ Implementation: MainWindow
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "mainwindow.h"

MainWindow::MainWindow()
{
  opt=new GetOpt();
  settings=0;
  setupUi((QMainWindow *)this);

  tree_view=new VarTreeView();
  tmodel=new VarTreeModel();

  splitter = new QSplitter(Qt::Horizontal,this);
  splitter2 = new QSplitter(Qt::Vertical);

  root=new VarList("Vision System");

  opts=new RenderOptions();

  QString stack_id="";

  //opt->addArgument("stack",&stack_id);
  opt->parse();

  //load RoboCup SSL stack by default:
  multi_stack=new MultiStackRoboCupSSL(opts, 2);

  VarExternal * stackvar;
  root->addChild(stackvar= new VarExternal((multi_stack->getSettingsFileName() + ".xml").c_str(),multi_stack->getName()));
  stackvar->addChild(multi_stack->getSettings());
  //create tabs, GL visualizations and tool-panes for each capture thread in the multi-stack:
  for (unsigned int i=0;i<multi_stack->threads.size();i++) {
    VisionStack * s = multi_stack->threads[i]->getStack();

    GLWidget * gl=new GLWidget();
    gl->setRingBuffer(multi_stack->threads[i]->getFrameBuffer());

    QString label = "Camera " + QString::number(i);

    VarList * threadvar = new VarList(label.toStdString());

    threadvar->addChild(s->getSettings());
    //iterate through plugin variables

    VideoWidget * w=new VideoWidget(label,gl);
    display_widgets.push_back(gl);
    threadvar->addChild(multi_stack->threads[i]->getSettings());
    //iterate through all plugins
    unsigned int n=s->stack.size();
    for (unsigned int j=0;j<n;j++) {
      VisionPlugin * p=s->stack[j];
      if (p->isSharedAmongStacks() && i==0) {
        //this is a shared global plugin...
        //add it to global pane
        stackvar->addChild(p->getSettings());
      } else {
        threadvar->addChild(p->getSettings());
        //this is a local plugin relating only to a single thread
        //add it to the context.
      }
    }

    stackvar->addChild(threadvar);

    splitter2->addWidget(w);
  }

  //FINISHED STRUCTURAL TREE
  //NOW LOAD DATA:

  world.push_back(root); 
  world=VarXML::read( world,"settings.xml");

  multi_stack->start();

  tree_view->setModel(tmodel);
  tmodel->setRootItems(world);
  tree_view->expandAndFocus(stackvar);
  tree_view->fitColumns();

  left_tab=new QTabWidget();
  left_tab->setTabPosition(QTabWidget::West);
  right_tab=new QTabWidget();
  right_tab->setTabPosition(QTabWidget::East);

  splitter->addWidget(left_tab);
  splitter->addWidget(splitter2);
  splitter->addWidget(right_tab);

  left_tab->addTab(tree_view,"Data-Tree");

  setCentralWidget(splitter); //was splitter

  startTimer(10);

}

void MainWindow::timerEvent( QTimerEvent * e) {
  (void)e;
  unsigned int n = display_widgets.size();
  RealTimeDisplayWidget * w;
  bool frame_changed;
  FrameBuffer * fb;

  for (unsigned int i=0;i<n;i++) {
    w = display_widgets[i];
    frame_changed=false;
    fb=w->getRingBuffer();
    if (fb!=0) {
      fb->lockRead();
      int cur=fb->curRead();
      if (fb->nextRead(false) != cur) frame_changed=true;
      fb->unlockRead();
    }
    w->displayLoopEvent(frame_changed,opts);
  }
}

void MainWindow::init() {

}

void MainWindow::closeEvent(QCloseEvent * event ) {
  (void)event;
  delete this;
}

MainWindow::~MainWindow() {
	//FIXME: right now we don't clean up anything
  VarXML::write(world,"settings.xml");

  multi_stack->stop();
      exit(0);
  delete opt;


}
