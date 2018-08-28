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
  \file    videowidget.h
  \brief   C++ Interface: VideoWidget
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef VIDEOWIDGET_H_
#define VIDEOWIDGET_H_

#include <QToolBar>
#include <QCloseEvent>
#include <QSplitter>
#include "ui_videowidget.h"
#include "videostats.h"
#include "ringbuffer.h"
#include "framedata.h"

/*!
  \class   VideoWidget
  \brief   A QT widget container that allows detaching, and fullscreen mode
  \author  Stefan Zickler, (C) 2008
*/
class VideoWidget : public QWidget, public Ui_VideoWidget {
    Q_OBJECT
  protected:
    QWidget * parent_backup;
    QAction * actionWindow;
    QAction * actionFullscreen;
    QWidget * _vis; //our child-widget (e.g. GlWidget or HistWidget)
    void closeEvent(QCloseEvent * event);
    void focusInEvent(QFocusEvent * event);

  public:
    VideoWidget(QString title = "Unnamed", QWidget * vis = 0);
    virtual ~VideoWidget();

  public slots:
    void processVideoStats(VideoStats stats);
    void toggleOn(bool val);
    void toggleWindow(bool val);
    void toggleFullScreen(bool val);

};

#endif /*VIDEOWIDGET_H_*/
