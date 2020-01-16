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
  \file    glLUTwidget.h
  \brief   C++ Interface: GLLUTWidget
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#ifndef GLLUTWIDGET_H_
#define GLLUTWIDGET_H_
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
#include <QWidgetAction>
#include "texture.h"
#include "glcamera.h"
#include "lut3d.h"
#include <QMutex>
#include <deque>
using namespace std;

/*!
  \class   GLLUTWidget
  \brief   An OpenGL-based editor for 3D Color LUTs of type LUT3D
  \author  Stefan Zickler, (C) 2008
*/
class GLLUTWidget : public QGLWidget, public RealTimeDisplayWidget
{
  Q_OBJECT
  enum ViewMode {
    VIEW_SINGLE,
    VIEW_GRID,
    VIEW_CUBE
  };

  class UndoState
  {
    public:
      lut_mask_t * table;
      UndoState(LUT3D * lut) {
        table=new lut_mask_t[lut->LUT_SIZE];
        memcpy(table,lut->getPointerPreshrunk(0,0,0),lut->LUT_SIZE);
      }

      ~UndoState() {
        delete[] table;
      };
      void restore(LUT3D * lut) {
        memcpy(lut->getPointerPreshrunk(0,0,0),table,lut->LUT_SIZE);
      }
  };

  class EditState
  {
    enum DrawMode {
      DRAW_ADD,
      DRAW_REMOVE,
    };
  public:
    EditState()
    {
      reset();
    }
    void reset()
    {
      slice_idx=0;
      channel=0;
      draw_exclusive=false;
      draw_mode=DRAW_ADD;
      drag_on=false;
      continuing_undo=false;
    }
    int      slice_idx;
    int      channel;
    bool     draw_exclusive;
    bool     continuing_undo;
    DrawMode draw_mode;
    bool     drag_on;
    int      drag_start_x;
    int      drag_start_y;
  };

  class Slice
  {
  public:
    rgbaTexture * bg;
    rgbaTexture * selection;
    rgbaTexture * sampler;
    int sampler_scale;
    bool sampler_update_pending;
    bool bg_update_pending;
    bool selection_update_pending;
    Slice()
    {
      sampler_scale=5;
      bg=new rgbaTexture("",true,false);
      selection=new rgbaTexture("",true,false);
      sampler=new rgbaTexture("",true,false);
      sampler_update_pending=false;
      bg_update_pending=false;
      selection_update_pending=false;
    };
    ~Slice()
    {
      delete bg;
      delete selection;
      delete sampler;
    }
    void resize(int w, int h)
    {
      bg->surface.allocate(w,h);
      bg->surface.fillBlack();
      selection->surface.allocate(w,h);
      selection->surface.fillBlack();
      sampler->surface.allocate(w*sampler_scale,h*sampler_scale);
      sampler->surface.fillBlack();
    }
  };



protected:
  deque<UndoState *> undoStack;
  deque<UndoState *> redoStack;
  QAction * actionRedo;
  QAction * actionUndo;
  QAction * actionExclusiveMode;
  QAction * actionViewToggleBackground;
  QAction * actionViewToggleOtherChannels;
  protected slots:
  void editUndo(); //performs an undo
  void editRedo(); //performs a redo
  void editStore(); //records a state for undoing (will also clear the redo stack)
  void editClearAll(); //clears both the redo and undo buffer
protected:

  LUTChannelMode _mode;
  QMutex m;
  GLint viewport[4];
  GLdouble projMatrix[16];
  GLdouble modelMatrix[16];
  void drawLine(int x0, int y0, int x1, int y1,QMouseEvent * event);
  void drawPixel(int x, int y,QMouseEvent * event);
  void drawSinglePixel(int x, int y,QMouseEvent * event);
  void setupProjection();
  bool glPixel2OrthoCoordinates(QMouseEvent * event, float & x, float & y);
  bool glPixel2LUTCoordinates(QMouseEvent * event,int & x, int &y);
  void drawSample(int i, int x, int y);
  inline rgba maskToRGBA(LUT3D * lut, const lut_mask_t & m);
  void drawEvent ( QMouseEvent * event );
  public:
  void setChannel(int c);
  EditState state;
  ViewMode view_mode;
  bool needs_init;
  bool gl_ready;
  void glDrawSlice(Slice * s);
  GLCamera cam;
  void drawSlice(Slice * s, LUT3D * lut, int idx, bool draw_bg=true, bool draw_selection=true, bool draw_sampler=true);
  vector<Slice *> slices;
  LUT3D * _lut;
  int vpW;
  int vpH;
  void samplePixel(const yuv & color);
  void add_del_Pixel(yuv color, bool add, bool continuing_undo);
  virtual void initializeGL();
  virtual void paintGL();
  virtual void resizeGL(int width, int height);
  void wheelEvent ( QWheelEvent * event );
  void keyPressEvent ( QKeyEvent * event );

  void mousePressEvent ( QMouseEvent * event );
  void mouseReleaseEvent ( QMouseEvent * event );
  void mouseMoveEvent ( QMouseEvent * event );
  void paintEvent(QPaintEvent * e);
  rgbaTexture * texture;

public:
  void setLUT(LUT3D * lut);
  bool copyLUT(lut_mask_t *pDataLUT, int size_copy, int color_index=-1);
  virtual void init();

  VideoStats stats;
  FrameCounter c_draw;

  void setObjectName(const QString & s)
  {
    QGLWidget::setObjectName(s);
  }

  void sampleImage(const RawImage & img);

  GLLUTWidget(LUTChannelMode mode, QWidget *parent = 0);
  virtual ~GLLUTWidget();

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


  virtual void timerEvent(QTimerEvent *)
  {
    redraw();
  }
public slots:
  virtual void redraw()
  {
    updateGL();
  }

public slots:
  void callZoomNormal();
  void callZoomFit();
  void callHelp();
  void switchMode();
  //void saveImage();
  void rebuildAndRedraw() {
    for (unsigned int i=0;i<slices.size();i++) {
      drawSlice(slices[i],_lut,i,false,true,false);
      redraw();
    }
  }
  void clearSampler();
signals:
  void updateVideoStats(VideoStats);
  void signalKeyPressEvent ( QKeyEvent * event );
};

#endif /*GLWIDGET_H_*/
