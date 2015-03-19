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
  \file    glwidget.h
  \brief   C++ Interface: GLWidget
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#ifndef GLWIDGET_H_
#define GLWIDGET_H_


#include <QtOpenGL/QGLWidget>
#include <QTime>
#include <QMutex>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include "image.h"
#include "videostats.h"
#include "ringbuffer.h"
#include "framedata.h"
#include "framecounter.h"
#include "zoom.h"
#include "realtimedisplaywidget.h"
#include "colorpicker.h"
#include "visionstack.h"
#include <QWidgetAction>
#include "plugin_visualize.h"

/*!
  \class   GLWidget
  \brief   An OpenGL-based real-time video display widget
  \author  Stefan Zickler, (C) 2008
*/
class GLWidget : public QGLWidget, public RealTimeDisplayWidget
{
  Q_OBJECT

protected:
  bool ALLOW_QPAINTER;
  int vpW;
  int vpH;
  long long last_frame;
  VisionStack * stack;
  QPoint mouseStart;
  double mouseStartPanX;
  double mouseStartPanY;
  QAction * actionFlipV;
  QAction * actionFlipH;
  QAction * actionBB;
  QAction * actionOn;
  QWidgetAction * actionColorPicker;
  //ColorPicker * colorPicker;
  virtual void initializeGL();
  virtual void paintGL();
  virtual void resizeGL(int width, int height);
  void wheelEvent ( QWheelEvent * event );
  void keyPressEvent ( QKeyEvent * event );
  void mouseAction ( QMouseEvent * event, pixelloc loc);
  void mousePressEvent ( QMouseEvent * event );
  void mouseReleaseEvent ( QMouseEvent * event );
  void mouseMoveEvent ( QMouseEvent * event );
  void paintEvent(QPaintEvent * e);

  RingBuffer<FrameData> * rb_bb;

public:
  virtual QSize sizeHint() const {
    QSize size;
    size.setHeight(580);
    size.setWidth(600);
    return size;
  }
  virtual void setRingBufferBB(RingBuffer<FrameData> * rb)
  {
    rb_bb=rb;
  }

  Zoom zoom;
  VideoStats stats;
  FrameCounter c_draw;
  FrameCounter c_loop;

  void setVisionStack(VisionStack * _stack)
  {
    stack=_stack;
  }
  void setObjectName(const QString & s)
  {
    QGLWidget::setObjectName(s);
  }
  GLWidget(QWidget *parent = 0, bool allow_qpainter_overlay=true);
  virtual ~GLWidget();

  virtual void focusInEvent ( QFocusEvent * event )
  {
    (void)event;
    if (parentWidget()!=0) {
      parentWidget()->setBackgroundRole(QPalette::Highlight);
      parentWidget()->setForegroundRole(QPalette::HighlightedText);
      parentWidget()->update();
    }
  }

  virtual void focusOutEvent ( QFocusEvent * event )
  {
    (void)event;
    if (parentWidget()!=0) {
      parentWidget()->setBackgroundRole(QPalette::Window);
      parentWidget()->setForegroundRole(QPalette::WindowText);
      parentWidget()->update();
    }
  }

  void mainDraw();
  virtual void myQPainterOverlay(QPainter & painter, QTransform & trans);
  virtual void myGLinit();
  virtual void myGLdraw();
  void setupViewPort ( int width, int height );


  virtual void displayLoopEvent(bool frame_changed, RenderOptions * opts)
  {
    (void)opts;
    bool changed=false;
    bool changed2=false;
    c_loop.count();

    if (frame_changed && rb!=0) {
      rb->lockRead();
      int cur=rb->curRead();
      last_frame=rb->getPointer(cur)->number;

      FrameData * frame = rb->getPointer(cur);
      CaptureStats * cstats = (CaptureStats *)frame->map.get("capture_stats");
      if (cstats != 0) {
        stats.capture_stats=(*cstats);
      }

      rb->unlockRead();
      redraw();
    }

    stats.fps_loop= c_loop.getFPS(changed);
    stats.fps_draw= c_draw.getFPS(changed2);
    if (changed || changed2)
      updateVideoStats(stats);
  }

  virtual void redraw()
  {
    if (ALLOW_QPAINTER) {
      repaint();
    } else {
      updateGL();
    }
  }

public slots:
  void flipImage();
  void callZoomNormal();
  void callZoomFit();
  void callHelp();
  void saveImage();

signals:
  void updateVideoStats(VideoStats);
  void signalMouseAction ( QMouseEvent * event, pixelloc loc );
  void signalKeyPressEvent ( QKeyEvent * event );
};

#endif /*GLWIDGET_H_*/
