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
  \file    videowidget.cpp
  \brief   C++ Implementation: VideoWidget
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "videowidget.h"

VideoWidget::VideoWidget(QString title, QWidget * vis)
{
  setupUi(this);
  this->setWindowTitle(title);
  titleLabel->setText(title);

  _vis=vis;

  //create a toolbar and add all the actions:
  QToolBar * toolbar=new QToolBar();

  actionWindow = new QAction(this);
  actionWindow->setObjectName("actionWindow");
  actionWindow->setCheckable(true);
  actionWindow->setIcon(QIcon(":/icons/window-new.png"));
  actionWindow->setShortcut(QKeySequence("Ctrl+w"));
  actionWindow->setToolTip("Toggle separate window (Ctrl-w)");
  actionWindow->setShortcutContext(Qt::WidgetShortcut);
  if (vis!=0)
    vis->addAction(actionWindow);
  connect(actionWindow, SIGNAL(triggered(bool)), this, SLOT(toggleWindow(bool)));

  actionFullscreen = new QAction(this);
  actionFullscreen->setObjectName("actionFullscreen");
  actionFullscreen->setCheckable(true);
  actionFullscreen->setIcon(QIcon(":/icons/view-fullscreen.png"));
  actionFullscreen->setShortcut(QKeySequence("Ctrl+f"));
  actionFullscreen->setToolTip("Toggle Fullscreen (Ctrl-f)");
  actionFullscreen->setShortcutContext(Qt::WidgetShortcut);
  if (vis!=0)
    vis->addAction(actionFullscreen);
  connect(actionFullscreen, SIGNAL(triggered(bool)), this, SLOT(toggleFullScreen(bool)));


  toolbar->setIconSize(QSize(16,16));
  toolbar->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );


  if (vis!=0) {
    //there is a sub-widget.
    //let's get its actions:
    QList<QAction *> toolactions=vis->actions();
    //reverse the list so the icons appear in the right order:
    // (qt really needs a list.reverse() function....)
    for (int i=0;i<toolactions.size()/2;i++) {
      toolactions.swap(i,toolactions.size()-(i+1));
    }

    //add the actions to the toolbar
    toolbar->addActions(toolactions);

    //add the widget itself and set it up:
    ((QVBoxLayout *)videoframe->layout())->insertWidget(1,vis);
    vis->setFocusPolicy(Qt::StrongFocus);
    vis->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
    connect(vis, SIGNAL(updateVideoStats(VideoStats)), this, SLOT(processVideoStats(VideoStats)));
  }

  videoframe->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
  this->bigframe->layout()->addWidget(toolbar);

  parent_backup=0;
}


void VideoWidget::toggleFullScreen(bool val)
{
  //this function takes care of fullscreening this widget
  //it will automatically do the nasties such as
  //reparenting etc...
  QWidget * op=this;
  if (val) {
    if (op->parentWidget()!=NULL) {
      parent_backup=op->parentWidget();
      op->setParent(NULL, Qt::Window);
    }
    op->showFullScreen();
    actionWindow->setChecked(false);
  } else {
    if (op->parentWidget()==NULL) {
      if (actionWindow->isChecked()==true) {
        op->showNormal();
      } else {
        if (parent_backup!=NULL) {
          QSplitter * sp = qobject_cast<QSplitter *>(parent_backup);
          if (sp!=0) {
            sp->addWidget(op);
          } else {
            if (parent_backup->layout()!=0) {
              parent_backup->layout()->addWidget(op);
            }
          }
        }
      }
    }
  }
  op->setFocus(Qt::OtherFocusReason);
}

VideoWidget::~VideoWidget()
{}

void VideoWidget::toggleOn(bool val)
{
  (void) val;
}

void VideoWidget::toggleWindow(bool val)
{
  (void) val;
  //this function takes care of detaching/attaching this widget
  //to become a separate window
  //it will automatically do the nasties such as
  //reparenting etc...
  QWidget * op=this;

  if (val) {
    QPoint pos = this->mapToGlobal(QPoint(0,0));
    if (op->parentWidget()!=NULL) {
      parent_backup=op->parentWidget();
      op->setParent(NULL, Qt::Window);
    }

    //op->setGeometry(,480);
    op->move(pos.x(),pos.y());
    op->showNormal();

  } else {
    if (op->parentWidget()==NULL) {
      if (parent_backup!=NULL) {
        QSplitter * sp = qobject_cast<QSplitter *>(parent_backup);
        if (sp!=0) {
          sp->addWidget(op);
        } else {
          if (parent_backup->layout()!=0) {
            parent_backup->layout()->addWidget(op);
          }
        }
      }
    }
  }
  actionFullscreen->setChecked(false);
  op->setFocus(Qt::OtherFocusReason);
}

void VideoWidget::focusInEvent ( QFocusEvent * event )
{
  (void)event;
  //forward the focus to the actual widget that we contain
  if (_vis!=0)
    _vis->setFocus(Qt::OtherFocusReason);
}

void VideoWidget::closeEvent ( QCloseEvent * event )
{
  //if this window is closed...simply re-integrate it into the parent
  actionWindow->trigger();
  event->ignore();
}

void VideoWidget::processVideoStats(VideoStats stats)
{
  //our display-widget as thrown us a stat-update event
  //let's display it
  statLabel->setText(
    "Capture: "+ QString::number(stats.capture_stats.fps_capture,'f',2)  + " fps | Display: " + QString::number(stats.fps_draw,'f',2) + " fps | "
    + QString::number(stats.fps_loop,'f',2) + " its/s");
}
