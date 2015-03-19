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
  \file    glLUTwidget.cpp
  \brief   C++ Implementation: GLLUTWidget
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#include "glLUTwidget.h"

#define UNDO_SIZE 20

void GLLUTWidget::editUndo() {
  if (_lut==0) return;
  if (undoStack.empty() == false) {
    UndoState * s=undoStack.back();
    UndoState * r=new UndoState(_lut);
    s->restore(_lut);
    for(int i=0; i<slices.size(); i++)
      drawSlice(slices[i],_lut, i, false,true,false);
    undoStack.pop_back();
    delete s;
    redoStack.push_back(r);
  }
  actionUndo->setEnabled(!undoStack.empty());
  actionRedo->setEnabled(!redoStack.empty());
  _lut->updateDerivedLUTs();
  redraw();
}

void GLLUTWidget::editRedo() {
  if (_lut==0) return;
  if (redoStack.empty() == false) {
    UndoState * s=redoStack.back();
    UndoState * r=new UndoState(_lut);
    s->restore(_lut);
    for(int i=0; i<slices.size(); i++)
      drawSlice(slices[i],_lut, i, false,true,false);
    redoStack.pop_back();
    delete s;
    undoStack.push_back(r);
  }
  actionUndo->setEnabled(!undoStack.empty());
  actionRedo->setEnabled(!redoStack.empty());
  _lut->updateDerivedLUTs();
  redraw();
}

void GLLUTWidget::editStore() {
  if (_lut==0) return;
  while(redoStack.empty()==false) {
    UndoState * s = redoStack.back();
    redoStack.pop_back();
    delete s;
  }
  while (undoStack.size() > UNDO_SIZE) {
    UndoState * f = undoStack.front();
    undoStack.pop_front();
    delete f;
  }
  undoStack.push_back(new UndoState(_lut));
  actionUndo->setEnabled(!undoStack.empty());
  actionRedo->setEnabled(!redoStack.empty());
  redraw();
}

void GLLUTWidget::editClearAll() {
  while(redoStack.empty()==false) {
    UndoState * s = redoStack.back();
    redoStack.pop_back();
    delete s;
  }
  while(undoStack.empty()==false) {
    UndoState * s = undoStack.back();
    undoStack.pop_back();
    delete s;
  }
  actionUndo->setEnabled(false);
  actionRedo->setEnabled(false);
}


bool GLLUTWidget::glPixel2OrthoCoordinates(QMouseEvent * event, float & x, float & y)
{
  double winX=(double)event->x();
  double winY=(double)vpH-event->y();

  GLdouble objx;
  GLdouble objy;
  GLdouble objz;
  GLfloat zerodepth=0.0;

  if (GL_TRUE==gluUnProject(winX, winY, zerodepth, modelMatrix, projMatrix, viewport, &objx, &objy, &objz)) {
    x=objx;
    y=objy;
    return true;
  } else {
    return false;
  }
}

bool GLLUTWidget::glPixel2LUTCoordinates(QMouseEvent * event,int & x, int &y) {
  if (_lut!=0) {
    float tx;
    float ty;
    if (glPixel2OrthoCoordinates(event,tx,ty)) {
      if (tx >= 0.0 && tx <= 1.0 && ty >= 0.0 && ty <= 1.0) {
        x=(int)((tx * (float)_lut->getSizeY()));
        y=(int)((ty * (float)_lut->getSizeZ()));
        //x=(int)(tx * 255.0);
        //y=(int)(ty * 255.0);
        if (x < 0) x=0;
        if (y < 0) y=0;
        if (x > _lut->getMaxY()) x=_lut->getMaxY();
        if (y > _lut->getMaxZ()) y=_lut->getMaxZ();
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}


void GLLUTWidget::drawLine( int x0, int y0, int x1, int y1 , QMouseEvent * event) {
  int x, y, dx, dy, sx, sy, ax, ay, decy, decx;
  x = x0;
  y = y0;
  dx = x1 - x0;
  dy = y1 - y0;
  if ( dx > 0 ) {
    sx = 1;
  } else {
    if ( dx < 0 ) {
      sx = -1;
      dx = -dx;
    } else {
      sx = 0;
    }
  }
  if ( dy > 0 ) {
    sy = 1;
  } else {
    if ( dy < 0 ) {
      sy = -1;
      dy = -dy;
    } else {
      sy = 0;
    }
  }
  ax = 2 * dx;
  ay = 2 * dy;
  if ( dy <= dx ) {
    for ( decy = ay - ax; ; x = x + sx, decy = decy + ay ) {
      drawPixel(x,y,event);
      //TABLE [_curstep] [ x ][ y ] = color;
      if ( x == x1 ) break;
      if ( decy >= 0 ) {
        decy = decy - ax;
        y = y + sy;
      }
    }
  } else {
    for ( decx = ax - ay; ; y = y + sy, decx = decx + ax ) {
      drawPixel(x,y,event);

      if ( y == y1 ) break;
      if ( decx >= 0 ) {
        decx = decx - ay;
        x = x + sx;
      }
    }
  }

}


void GLLUTWidget::drawPixel(int x, int y, QMouseEvent * event) {
  //this will draw 9 pixels at the same time
  if ((event->modifiers() & Qt::ControlModifier) != 0x00) {
    if (x > 0) {
      drawSinglePixel(x-1,y,event);
      if (y > 0)  drawSinglePixel(x-1,y-1,event);
      if (y < _lut->getMaxZ()) drawSinglePixel(x-1,y+1,event);
    }
    if (x < _lut->getMaxY()) {
      drawSinglePixel(x+1,y,event);
      if (y > 0)  drawSinglePixel(x+1,y-1,event);
      if (y < _lut->getMaxZ()) drawSinglePixel(x+1,y+1,event);
    }
    if (y > 0) drawSinglePixel(x,y-1,event);
    if (y < _lut->getMaxZ()) drawSinglePixel(x,y+1,event);
  }
  drawSinglePixel(x,y,event);
}

void GLLUTWidget::drawSinglePixel(int x, int y, QMouseEvent * event) {
    lut_mask_t mask;
    mask=_lut->get_preshrunk(state.slice_idx,x,y);
    //perform drawing ops on mask value
    //redraw texture
    //check whether we are in removal mode:
    if ((event->modifiers() & Qt::AltModifier) != 0x00) {
      //remove all
      mask=0x00;
    } else if ((event->modifiers() & Qt::ShiftModifier) != 0x00) {
      //remove current channel
      if (_mode==LUTChannelMode_Numeric) {
        mask=0x00;
      } else {
        mask=mask & (~(0x01 << state.channel));
      }
    } else {
      if (_mode==LUTChannelMode_Numeric) {
        mask=state.channel;
      } else {
        if (actionExclusiveMode->isChecked()) {
          mask=(0x01 << state.channel);
        } else {
          mask|=(0x01 << state.channel);
        }
      }
    }

    _lut->set_preshrunk(state.slice_idx,x,y,mask);

    if (_mode==LUTChannelMode_Numeric) {
      slices[state.slice_idx]->selection->surface.setPixel(x,y,maskToRGBA(_lut,mask));
    } else {
      if (actionViewToggleOtherChannels->isChecked()==false) {
        slices[state.slice_idx]->selection->surface.setPixel(x,y,maskToRGBA(_lut,(0x01 << state.channel) & mask));
      } else {
        slices[state.slice_idx]->selection->surface.setPixel(x,y,maskToRGBA(_lut,mask));
      }
    }

}



void GLLUTWidget::setChannel(int c) {
  state.channel=c;
  if (actionViewToggleOtherChannels->isChecked()==false) {
    for (unsigned int i=0;i<slices.size();i++) {
      drawSlice( slices[i],_lut,i,false,true,false);
    }
  }
  redraw();
}


void GLLUTWidget::drawEvent ( QMouseEvent * event )
{
  
 //get coordinates
  int x;
  int y;
  if (glPixel2LUTCoordinates(event,x,y)==true) {
    _lut->lock();
    if ((event->button() == Qt::RightButton)) {
      //if right mouse button:
      //FIXME: add a (if direct statement)
      
        if (state.continuing_undo==false) editStore();
        _lut->maskFillYZ(state.slice_idx, x,y,
                          (_mode==LUTChannelMode_Numeric) ? state.channel : (((event->modifiers() & Qt::AltModifier) != 0x00) ? (~(0x00)) : (0x01 << state.channel)),
                          _mode,
                           (event->modifiers() & (Qt::ShiftModifier | Qt::AltModifier)) != 0x00,actionViewToggleOtherChannels->isChecked(),actionExclusiveMode->isChecked());
        drawSlice(slices[state.slice_idx], _lut, state.slice_idx,false, true,false);

    //update texture
    slices[state.slice_idx]->selection_update_pending=true;
      state.continuing_undo=false;
    } else {
      if ((event->buttons() & Qt::RightButton) == 0x00) {
        if (state.continuing_undo==false) editStore();
        //get current mask value
        if (state.drag_on==true) {
          drawLine(x,y,state.drag_start_x,state.drag_start_y,event);
        } else {
          drawPixel(x,y,event);
        }
        state.drag_start_x=x;
        state.drag_start_y=y;
        state.drag_on=true;
        state.continuing_undo=true;
      }
    }
    //update texture
    slices[state.slice_idx]->selection_update_pending=true;
    _lut->unlock();
    
    this->redraw();
  }
}

void GLLUTWidget::add_del_Pixel(yuv color, bool add, bool continuing_undo)
{
  _lut->lock();

  //compute slice it sits on:
  int i=_lut->norm2lutX(color.y);
  if (i >= 0 && i < (int)slices.size())
    state.slice_idx = i;

  if(!continuing_undo)
    editStore();
  Qt::KeyboardModifiers mod = add ? Qt::NoModifier : Qt::ShiftModifier;
  drawSinglePixel(_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v),
                  new QMouseEvent(QEvent::None,QPoint(),Qt::NoButton,Qt::NoButton,mod));

  slices[state.slice_idx]->selection_update_pending=true;
  _lut->unlock();

  this->redraw();
}

void GLLUTWidget::mousePressEvent ( QMouseEvent * event )
{
  if (view_mode==VIEW_CUBE) {
    cam.mousePressEvent(event);
  } else {
    state.drag_on=false;
    drawEvent(event);
  }
  redraw();
}

void GLLUTWidget::mouseReleaseEvent ( QMouseEvent * event )
{
  if (view_mode==VIEW_CUBE) {
    cam.mouseReleaseEvent(event);
  } else {
    //editStore();
    //drawEvent(event);
    state.drag_on=false;
    state.continuing_undo=false;
    _lut->updateDerivedLUTs();
  }
  redraw();
}

void GLLUTWidget::mouseMoveEvent ( QMouseEvent * event )
{
  if (view_mode==VIEW_CUBE) {
    cam.mouseMoveEvent(event);
  } else {
    drawEvent(event);
  }
  redraw();
}


GLLUTWidget::GLLUTWidget(LUTChannelMode mode, QWidget *parent) : QGLWidget(parent)
{
  rb=0;
  _lut=0;
  _mode=mode;
  needs_init=false;
  gl_ready=false;
  view_mode=VIEW_SINGLE;
  setFocusPolicy(Qt::StrongFocus);
  cam.reset();
  cam.setPitch( -0.785398163);
  cam.setYaw( -0.785398163);
  cam.setDistance(6.0);


  setMinimumSize(60,40);

  /*QAction * actionSave = new QAction(this);
  actionSave->setObjectName("actionSave");
  actionSave->setIcon(QIcon(":/icons/document-save.png"));
  actionSave->setShortcut(QKeySequence("Ctrl+s"));
  actionSave->setToolTip("Save Image... (Ctrl-s)");
  actionSave->setShortcutContext(Qt::WidgetShortcut);
  addAction(actionSave);*/

  QAction * actionHelp = new QAction(this);
  actionHelp->setObjectName("actionHelp");
  actionHelp->setIcon(QIcon(":/icons/help-contents.png"));
  actionHelp->setShortcut(QKeySequence("Ctrl+h"));
  actionHelp->setToolTip("Show keyboard shortcuts (Ctrl-h)");
  actionHelp->setShortcutContext(Qt::WidgetShortcut);
  addAction(actionHelp);

  /*QAction * actionZoomFit = new QAction(this);
  actionZoomFit->setObjectName("actionZoomFit");
  actionZoomFit->setIcon(QIcon(":/icons/zoom-best-fit.png"));
  actionZoomFit->setShortcut(QKeySequence("Space"));
  actionZoomFit->setToolTip("Zoom to Fit (Space)");
  actionZoomFit->setShortcutContext(Qt::WidgetShortcut);
  addAction(actionZoomFit);*/

  QAction * actionZoomNormal = new QAction(this);
  actionZoomNormal->setObjectName("actionZoomNormal");
  actionZoomNormal->setIcon(QIcon(":/icons/zoom-original.png"));
  actionZoomNormal->setShortcut(QKeySequence("Home"));
  actionZoomNormal->setShortcutContext(Qt::WidgetShortcut);
  actionZoomNormal->setToolTip("Zoom to 100% (Home)");
  addAction(actionZoomNormal);

  QAction * actionSwitchMode = new QAction(this);
  actionSwitchMode->setObjectName("actionSwitchMode");
  actionSwitchMode->setIcon(QIcon(":/icons/3d.png"));
  actionSwitchMode->setShortcut(QKeySequence("Home"));
  actionSwitchMode->setShortcutContext(Qt::WidgetShortcut);
  actionSwitchMode->setToolTip("Switch Mode");
  addAction(actionSwitchMode);

  actionViewToggleBackground = new QAction(this);
  actionViewToggleBackground->setObjectName("actionViewToggleBackground");
  actionViewToggleBackground->setIcon(QIcon(":/icons/background.png"));
  actionViewToggleBackground->setCheckable(true);
  actionViewToggleBackground->setChecked(true);
  actionViewToggleBackground->setShortcut(QKeySequence("b"));
  actionViewToggleBackground->setShortcutContext(Qt::WidgetShortcut);
  actionViewToggleBackground->setToolTip("Display Background");
  addAction(actionViewToggleBackground);

  actionViewToggleOtherChannels = new QAction(this);
  actionViewToggleOtherChannels->setObjectName("actionViewToggleOtherChannels");
  actionViewToggleOtherChannels->setIcon(QIcon(":/icons/colors.png"));
  actionViewToggleOtherChannels->setCheckable(true);
  actionViewToggleOtherChannels->setChecked(true);
  actionViewToggleOtherChannels->setShortcut(QKeySequence("o"));
  actionViewToggleOtherChannels->setShortcutContext(Qt::WidgetShortcut);
  actionViewToggleOtherChannels->setToolTip("Display All Channels");
  if (mode==LUTChannelMode_Bitwise) {
    addAction(actionViewToggleOtherChannels);
  }


  QAction * actionSep = new QAction(this);
  actionSep->setSeparator(true);
  addAction(actionSep);

  actionUndo = new QAction(this);
  actionUndo->setEnabled(false);
  actionUndo->setObjectName("actionUndo");
  actionUndo->setIcon(QIcon(":/icons/undo.png"));
  actionUndo->setShortcut(QKeySequence("Backspace"));
  actionUndo->setShortcutContext(Qt::WidgetShortcut);
  actionUndo->setToolTip("Undo");
  addAction(actionUndo);

  actionRedo = new QAction(this);
  actionUndo->setEnabled(false);
  actionRedo->setObjectName("actionRedo");
  actionRedo->setIcon(QIcon(":/icons/redo.png"));
  actionRedo->setShortcut(QKeySequence("Insert"));
  actionRedo->setShortcutContext(Qt::WidgetShortcut);
  actionRedo->setToolTip("Redo");
  addAction(actionRedo);

  QAction * actionSep2 = new QAction(this);
  actionSep2->setSeparator(true);
  addAction(actionSep2);


  actionExclusiveMode = new QAction(this);
  actionExclusiveMode->setObjectName("actionExclusiveMode");
  actionExclusiveMode->setIcon(QIcon(":/icons/xedit.png"));
  actionExclusiveMode->setCheckable(true);
  actionExclusiveMode->setShortcut(QKeySequence("x"));
  actionExclusiveMode->setShortcutContext(Qt::WidgetShortcut);
  actionExclusiveMode->setToolTip("Exclusive Channel Mode");
  if (mode==LUTChannelMode_Bitwise) {
    addAction(actionExclusiveMode);
  } else {
    actionExclusiveMode->setChecked(true);
  }

  QAction * actionClearSampler = new QAction(this);
  actionClearSampler->setObjectName("actionClearSampler");
  actionClearSampler->setIcon(QIcon(":/icons/xload.png"));
  actionClearSampler->setShortcut(QKeySequence("c"));
  actionClearSampler->setShortcutContext(Qt::WidgetShortcut);
  actionClearSampler->setToolTip("Clear Sampler");
  addAction(actionClearSampler);

  connect(actionViewToggleBackground, SIGNAL(triggered()), this, SLOT(redraw()));
  connect(actionViewToggleOtherChannels, SIGNAL(triggered()), this, SLOT(rebuildAndRedraw()));
  connect(actionSwitchMode, SIGNAL(triggered()), this, SLOT(switchMode()));
  
  connect(actionRedo, SIGNAL(triggered()), this, SLOT(editRedo()));
  connect(actionUndo, SIGNAL(triggered()), this, SLOT(editUndo()));
  connect(actionClearSampler, SIGNAL(triggered()), this, SLOT(clearSampler()));
  //connect(actionSave, SIGNAL(triggered()), this, SLOT(saveImage()));
  connect(actionHelp, SIGNAL(triggered()), this, SLOT(callHelp()));
  
  connect(actionZoomNormal, SIGNAL(triggered()), this, SLOT(callZoomNormal()));

  editClearAll();
}

GLLUTWidget::~GLLUTWidget()
{
  //delete texture;
}


void GLLUTWidget::initializeGL()
{
  qglClearColor(QColor(0,0,0));
  //DISABLE EVERYTHING
  //trying to make video-rendering as fast as possible
  //glShadeModel(GL_FLAT);
  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  //glEnable(GL_STENCIL_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glShadeModel(GL_FLAT);

  glAlphaFunc(GL_GREATER,0.001);
  glEnable(GL_ALPHA_TEST);
  glShadeModel(GL_SMOOTH);
  //Enable textures:
  glEnable( GL_TEXTURE_2D );
  //make lines smooth:
  glEnable(GL_LINE_SMOOTH);
  //enable depth testing

  //  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glDisable(GL_CULL_FACE);

  gl_ready=true;

}

void GLLUTWidget::paintGL()
{
  m.lock();
  if (gl_ready && needs_init && _lut!=0) {
    init();
  }
  m.unlock();

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

  setupProjection();

  //setup lens:
  
  //switch to modelview mode
  glMatrixMode( GL_MODELVIEW );
  
  glLoadIdentity();

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  //cam.apply();

  if (view_mode==VIEW_SINGLE || view_mode==VIEW_GRID) {
    gluLookAt(0.0,0.0,100.0, 0.0, 0.0, 0.0,0.0,1.0,0.0);
  } else {
    cam.apply();
  }

  if (_lut==0) return;
  //store matrices for reverse mouse-lookup
  for (int i=0;i<(int)slices.size();i++) {
    Slice * s=slices[i];
    if (s->bg_update_pending) {
      if (s->bg->update()==false) printf("draw-slice texture update failed\n");
      s->bg_update_pending=false;
    }
    if (s->selection_update_pending) {
      if (s->selection->update(GL_CLAMP,GL_CLAMP,GL_NEAREST,GL_NEAREST)==false) printf("draw-slice texture update failed\n");
      s->selection_update_pending=false;
    }
    if (s->sampler_update_pending) {
      if (s->sampler->update(GL_CLAMP,GL_CLAMP,GL_LINEAR,GL_NEAREST)==false) printf("draw-slice texture update failed\n");
      s->sampler_update_pending=false;
    }
  }

  
  //let's draw a slice
  //glColor3f(0.0,0.0,0.0);
  if (view_mode==VIEW_SINGLE) {
    glGetIntegerv(GL_VIEWPORT,viewport);
    glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
    glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
    glDrawSlice(slices[state.slice_idx]);
  } else if (view_mode==VIEW_GRID) {
    int cols=(int)(sqrt(slices.size()));
    int rows=slices.size()/cols;
    int c=0;
    int r=0;
    for (int i=0;i<(int)slices.size();i++) {
      glPushMatrix();
      glTranslatef((1.0/(float)cols)*(float)c,(1.0/(float)rows)*(float)r,0.0);
      c++;
      if (c >= cols) { c=0; r++; }

      glScalef((1.0/(float)cols)*0.98,(1.0/(float)rows)*0.98,1.0);
      glTranslatef(0.01,0.01,0.0);
      if (i==state.slice_idx) {
        glGetIntegerv(GL_VIEWPORT,viewport);
        glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
        glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
      }
      glDrawSlice(slices[i]);
      glBindTexture(GL_TEXTURE_2D,0);
      glColor3f(1.0,1.0,1.0);
      if (i==state.slice_idx) {
        glPushMatrix();
        glLineWidth(2.0);
        glBegin(GL_LINES);

        glVertex3f(0.0,0.0,0.01);
        glVertex3f(1.0,0.0,0.01);

        glVertex3f(1.0,0.0,0.01);
        glVertex3f(1.0,1.0,0.01);

        glVertex3f(1.0,1.0,0.01);
        glVertex3f(0.0,1.0,0.01);

        glVertex3f(0.0,1.0,0.01);
        glVertex3f(0.0,0.0,0.01);

        glEnd();
        glPopMatrix();
      }
      glPopMatrix();
    }
  } else if (view_mode==VIEW_CUBE) {

      

      glTranslatef(-0.5,-0.5,-0.5);
      glColor3f(0.7,0.7,0.7);
      glPushMatrix();
        glLineWidth(1.0);
        glBegin(GL_LINES);

        glVertex3f(0.0,0.0,0.0);
        glVertex3f(1.0,0.0,0.0);

        glVertex3f(0.0,1.0,0.0);
        glVertex3f(1.0,1.0,0.0);

        glVertex3f(0.0,0.0,1.0);
        glVertex3f(1.0,0.0,1.0);

        glVertex3f(0.0,1.0,1.0);
        glVertex3f(1.0,1.0,1.0);

        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,1.0,0.0);

        glVertex3f(1.0,0.0,0.0);
        glVertex3f(1.0,1.0,0.0);

        glVertex3f(0.0,0.0,1.0);
        glVertex3f(0.0,1.0,1.0);

        glVertex3f(1.0,0.0,1.0);
        glVertex3f(1.0,1.0,1.0);

        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,0.0,1.0);

        glVertex3f(0.0,1.0,0.0);
        glVertex3f(0.0,1.0,1.0);

        glVertex3f(1.0,0.0,0.0);
        glVertex3f(1.0,0.0,1.0);

        glVertex3f(1.0,1.0,0.0);
        glVertex3f(1.0,1.0,1.0);

        glEnd();
      glPopMatrix();
    for (int i=0;i<(int)slices.size();i++) {
      glPushMatrix();
      glTranslatef(0.0,0.0,(1.0 / (float)slices.size()) * (float)i);
      //glScalef((1.0/(float)cols)*0.98,(1.0/(float)rows)*0.98,1.0);
      //glTranslatef(0.01,0.01,0.0);
      /*if (i==state.slice_idx) {
        glGetIntegerv(GL_VIEWPORT,viewport);
        glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
        glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
      }*/
      glDrawSlice(slices[i]);
      glBindTexture(GL_TEXTURE_2D,0);
      


      glColor3f(1.0,1.0,1.0);
      if (i==state.slice_idx) {
        glPushMatrix();
        glLineWidth(2.0);
        glBegin(GL_LINES);

        glVertex3f(0.0,0.0,0.01);
        glVertex3f(1.0,0.0,0.01);

        glVertex3f(1.0,0.0,0.01);
        glVertex3f(1.0,1.0,0.01);

        glVertex3f(1.0,1.0,0.01);
        glVertex3f(0.0,1.0,0.01);

        glVertex3f(0.0,1.0,0.01);
        glVertex3f(0.0,0.0,0.01);

        glEnd();
        glPopMatrix();
      }
      glPopMatrix();
    }
  }

  c_draw.count();
  //updateVideoStats(stats);
}


void GLLUTWidget::glDrawSlice(Slice * s) {
  float z_offset=0.0001;
  float z=0.0;
  glPushMatrix();
  if (s!=0) {
    if (actionViewToggleBackground->isChecked()) {
      if (s->bg->bind()) {
      //printf("dimensions: %d %d\n",s->bg->surface.getWidth(),s->bg->surface.getHeight());
      //printf("color: %d %d %d %d\n",s->bg->surface.getPixel(0,0).r , s->bg->surface.getPixel(0,0).g,s->bg->surface.getPixel(0,0).b,s->bg->surface.getPixel(0,0).a);
      //glColor4d(255,0,0,128);

        glBegin(GL_QUADS);
          glTexCoord2f(0.0,0.0);
          glVertex3f(0.0  ,0.0,z);

          glTexCoord2f(0.0,1.0);
          glVertex3f(0.0,1.0  ,z);

          glTexCoord2f(1.0,1.0);
          glVertex3f  (1.0,1.0,z);

          glTexCoord2f(1.0,0.0);
          glVertex3f(1.0 ,0.0  ,z);
        glEnd();
      } else {
        printf("bind failed\n");
      }
    }
    if (s->selection->bind()) {
      glBegin(GL_QUADS);
        glTexCoord2f(0.0,0.0);
        glVertex3f(0.0  ,0.0,z+z_offset);

        glTexCoord2f(0.0,1.0);
        glVertex3f(0.0,1.0  ,z+z_offset);

        glTexCoord2f(1.0,1.0);
        glVertex3f  (1.0,1.0,z+z_offset);

        glTexCoord2f(1.0,0.0);
        glVertex3f(1.0 ,0.0  ,z+z_offset);
      glEnd();
    }
    glEnable(GL_COLOR_LOGIC_OP);
    //glLogicOp(GL_COPY_INVERTED);
    glLogicOp(GL_XOR);
    if (s->sampler->bind()) {
      glBegin(GL_QUADS);
        glTexCoord2f(0.0,0.0);
        glVertex3f(0.0  ,0.0,z+z_offset*2);

        glTexCoord2f(0.0,1.0);
        glVertex3f(0.0,1.0  ,z+z_offset*2);

        glTexCoord2f(1.0,1.0);
        glVertex3f  (1.0,1.0,z+z_offset*2);

        glTexCoord2f(1.0,0.0);
        glVertex3f(1.0 ,0.0  ,z+z_offset*2);
      glEnd();
    }
    glDisable(GL_COLOR_LOGIC_OP);

  }
  glPopMatrix();
}

void GLLUTWidget::setupProjection() {
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  if (view_mode==VIEW_CUBE) {
      //3D Mode:
    gluPerspective(35.0f,(double)vpW/(double)vpH,0.2,20.0);
    //gluPerspective(35.0f,1.0,0.2,20.0);
  } else {
    //ORTHO MODE:
    glOrtho(0, 1.0, 0, 1.0, -200.0, 200.0);
  }
}

void GLLUTWidget::resizeGL(int width, int height)
{

  //save new viewport dimensions:
  /*if (width < height) {
    vpW=vpH=width;
  } else {
    vpW=vpH=height;
  }*/
  vpW=width;
  vpH=height;
  if (vpW < 1) vpW=1;
  if (vpH < 1) vpH=1;

  //glViewport((width > vpW) ? ((width-vpW) / 2) : 0, (height> vpH) ? ((height-vpH) / 2) : 0, vpW, vpH);
  glViewport(0, 0, vpW, vpH);

  setupProjection();

}

inline rgba GLLUTWidget::maskToRGBA(LUT3D * lut, const lut_mask_t & m) {
  if (m==0) {
    return rgba(0,0,0,0);
  } else {
    rgb result(0,0,0);
    if (_mode==LUTChannelMode_Numeric) {
      result=lut->channels[m].draw_color;
    } else {
      for (int i=0;i<(int)lut->channels.size();i++) {
        if ((m & (0x01 << i)) != 0x00) {
          result+=lut->channels[i].draw_color;
        }
      }
    }
    return Conversions::rgb2rgba(result,255);
  }
}

void GLLUTWidget::drawSlice(Slice * s, LUT3D * lut, int idx, bool draw_bg, bool draw_selection, bool draw_sampler) {
  if (lut->getColorSpace()==CSPACE_YUV) {
    unsigned char  xn=lut->getSizeY();
    unsigned char  yn=lut->getSizeZ();
    //printf("size: %d   max: %d    bits: %d     shift: %d \n"
    //,lut->getSizeZ(),lut->getMaxZ(),lut->Z_BITS,lut->Z_SHIFT);
    for (unsigned char x=0;x<xn;x++) {
      for (unsigned char  y=0;y<yn;y++) {
        //printf("yn: %d\n",(yn/2)-1);
        //printf("lut: %d\n",lut->lut2normZ((yn/2)-1));
        if (draw_bg) s->bg->surface.setPixel( x,y,Conversions::rgb2rgba(Conversions::yuv2rgb(yuv(lut->lut2normX(idx),lut->lut2normY(x),lut->lut2normZ(y))),128));
        if (draw_selection) {
          if ((_mode==LUTChannelMode_Bitwise) && (actionViewToggleOtherChannels->isChecked()==false)) {
            s->selection->surface.setPixel( x,y,maskToRGBA(lut,(0x01 << state.channel) & lut->get_preshrunk(idx,x,y)));
          } else {
            s->selection->surface.setPixel( x,y,maskToRGBA(lut,lut->get_preshrunk(idx,x,y)));
          }
        }
        //s->bg->surface.setPixel( x,y,rgba(255,0,255,128));
      }
    }
    if (draw_bg) s->bg_update_pending=true;
    if (draw_selection) s->selection_update_pending=true;
    if (draw_sampler) s->sampler_update_pending=true;

  } else {
    printf("currently only YUV colorspace-luts are supported\n");
  }
}

void GLLUTWidget::setLUT(LUT3D * lut) {
  m.lock();
  _lut=lut;
  needs_init=true;
  m.unlock();
}

void GLLUTWidget::init() {
  if (_lut!=0) {
    int z=_lut->getSizeX();
    int x=_lut->getSizeY();
    int y=_lut->getSizeZ();
    //clear any old slices:
    unsigned int n=slices.size();
    if (n>0) {
      for (unsigned int i=n;i>0;i--) {
        delete slices[i-1];
      }
    }
    slices.clear();

    //reload 'slices' into texture memory:
    for (unsigned int i=0;i<(unsigned int)z;i++) {
      Slice * tmp=new Slice();
      tmp->resize(x,y);
      tmp->bg->load();
      tmp->selection->load(GL_CLAMP,GL_CLAMP,GL_NEAREST,GL_NEAREST);
      tmp->sampler->load(GL_CLAMP,GL_CLAMP, GL_LINEAR, GL_LINEAR);
      drawSlice(tmp,_lut,i);
      slices.push_back(tmp);
    }
    needs_init=false;
  }
}

void GLLUTWidget::wheelEvent ( QWheelEvent * event )
{
  if (view_mode==VIEW_CUBE) {
    cam.wheelEvent(event);
  } else {
    int delta=event->delta() / 120;
    if (delta == 0) delta=((event->delta() > 0) ? 1 : 0);
    state.slice_idx=max(min(state.slice_idx+delta,(int)slices.size()-1),0);
    event->accept();
  }
  redraw();
}

void GLLUTWidget::callZoomNormal()
{
  cam.reset();
  cam.setPitch( -0.785398163);
  cam.setYaw( -0.785398163);
  cam.setDistance(6.0);
  this->redraw(); //upgl
}

void GLLUTWidget::callZoomFit()
{

  cam.reset();
  cam.setPitch( -0.785398163);
  cam.setYaw( -0.785398163);
  cam.setDistance(6.0);
  this->redraw(); //upgl
}

void GLLUTWidget::callHelp()
{
  QMessageBox::information(this,"Keyboard and Mouse shortcuts",
                           "Left-Click: Draw\nRight-Click: Fill\nShift-Modifier: Erase Current Channel\nCtrl-Modifier: 9 pixel stroke\na / z or MouseWheel: Move through slices\nm: switch drawing (m)odes\nb: toggle (b)ackground\ni:  sample entire image\nc: clear sampler\n");
}

void GLLUTWidget::switchMode()
{
    if (view_mode==VIEW_SINGLE) {
      view_mode=VIEW_GRID;
    } else if (view_mode==VIEW_GRID) {
      view_mode=VIEW_CUBE;
    } else {
      view_mode=VIEW_SINGLE;
    }
    this->redraw(); //upgl
}
void GLLUTWidget::keyPressEvent ( QKeyEvent * event )
{
  if (event->key()==Qt::Key_Left) {
    //this->zoom.panRight();
    this->redraw(); //upgl
    event->accept();
  } else if (event->key()==Qt::Key_M) {
    switchMode();
  } else if (event->key()==Qt::Key_A) {
    if (state.slice_idx < (int)slices.size()-1) state.slice_idx++;
    this->redraw(); //upgl
  } else if (event->key()==Qt::Key_I) {
    signalKeyPressEvent(event);
    event->accept();
  } else if (event->key()==Qt::Key_C) {
    signalKeyPressEvent(event);
    event->accept();    
  } else if (event->key()==Qt::Key_Z) {
    if (state.slice_idx > 0) state.slice_idx--;
    this->redraw(); //upgl
  } else if (event->key()==Qt::Key_Right) {
    //this->zoom.panLeft();
    this->redraw(); //upgl
    event->accept();
  } else if (event->key()==Qt::Key_Up) {
    //this->zoom.panDown();
    this->redraw(); //upgl
    event->accept();
  } else if (event->key()==Qt::Key_Down) {
    //this->zoom.panUp();
    this->redraw(); //upgl
    event->accept();
  } else if (event->key()==Qt::Key_PageUp) {
    //this->zoom.zoomIn();
    this->redraw(); //upgl
    event->accept();
  } else if (event->key()==Qt::Key_PageDown) {
    //this->zoom.zoomOut();
    this->redraw(); //upgl
    event->accept();
  } else {
    event->ignore();
  }
}

void GLLUTWidget::samplePixel(const yuv & color) {
  //compute slice it sits on:
  int i=_lut->norm2lutX(color.y);
  if (i >= 0 && i < (int)slices.size()) {
    //old:
    //slices[i]->sampler->surface.setPixel(_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v),rgba(255,255,255,255));
    //new: draw an X
    drawSample(i,_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v));
    slices[i]->sampler_update_pending=true;
    redraw();
  }
  
}

void GLLUTWidget::drawSample(int i, int x, int y) {
    int scale=slices[i]->sampler_scale;
    x*=scale;
    y*=scale;

    for (int p = 0; p < scale; p++) {
      /*(slices[i]->sampler->surface.setPixel(x+p,y,rgba(0,0,0,255));
      slices[i]->sampler->surface.setPixel(x+p,y+scale,rgba(0,0,0,255));
      slices[i]->sampler->surface.setPixel(x,y+p,rgba(0,0,0,255));
      slices[i]->sampler->surface.setPixel(x+scale,y+p,rgba(0,0,0,255));*/
      slices[i]->sampler->surface.setPixel(x+p,y+p,rgba(255,255,255,255));
      slices[i]->sampler->surface.setPixel(x+(scale-(p+1)),y+p,rgba(255,255,255,255));
    }
}

void GLLUTWidget::sampleImage(const RawImage & img) {
  //compute slice it sits on:
  ColorFormat source_format=img.getColorFormat();
  
  int n=img.getNumPixels();
  
  yuv color;
  int i=0;
  
  if (img.getWidth() > 1 && img.getHeight() > 1) {
    if (source_format==COLOR_RGB8) {
      rgbImage rgb_img(img);
      rgb * color_rgb=rgb_img.getPixelData();
      for (int j=0;j<n;j++) {
        color=Conversions::rgb2yuv(*color_rgb);
        i=_lut->norm2lutX(color.y);
        if (i >= 0 && i < (int)slices.size()) {
          drawSample(i,_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v));
          //slices[i]->sampler->surface.setPixel(_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v),rgba(255,255,255,255));
          slices[i]->sampler_update_pending=true;
        }
        color_rgb++;
      }
    } else if (source_format==COLOR_YUV444) {    
      yuvImage yuv_img(img);
      yuv * color_yuv=yuv_img.getPixelData();
      for (int j=0;j<n;j++) {
        color=(*color_yuv);
        i=_lut->norm2lutX(color.y);
        if (i >= 0 && i < (int)slices.size()) {
          //slices[i]->sampler->surface.setPixel(_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v),rgba(255,255,255,255));
          drawSample(i,_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v));
          slices[i]->sampler_update_pending=true;
        }
        color_yuv++;
      }
    } else if (source_format==COLOR_YUV422_UYVY) {
        uyvy * color_uyvy = (uyvy*)img.getData();
        uyvy color_uyvy_tmp;
        for (int j=0;j<n;j+=2) {
          color_uyvy_tmp=(*color_uyvy);
          color.u=color_uyvy_tmp.u;
          color.v=color_uyvy_tmp.v;
  
          color.y=color_uyvy_tmp.y1;
          i=_lut->norm2lutX(color.y);
          if (i >= 0 && i < (int)slices.size()) {
            //slices[i]->sampler->surface.setPixel(_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v),rgba(255,255,255,255));
            drawSample(i,_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v));
            slices[i]->sampler_update_pending=true;
          }
  
          color.y=color_uyvy_tmp.y2;
          i=_lut->norm2lutX(color.y);
          if (i >= 0 && i < (int)slices.size()) {
            //slices[i]->sampler->surface.setPixel(_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v),rgba(255,255,255,255));
            drawSample(i,_lut->norm2lutY(color.u),_lut->norm2lutZ(color.v));
            slices[i]->sampler_update_pending=true;
          }
          color_uyvy++;
        }
    } else {
      fprintf(stderr,"Unable to sample colors from frame of format: %s\n",Colors::colorFormatToString(source_format).c_str());
      fprintf(stderr,"Currently supported are rgb8, yuv444, and yuv422 (UYVY).\n");
      fprintf(stderr,"(Feel free to add more conversions to glLUTwidget.cpp).\n");
    }
   }
  redraw();

}

void GLLUTWidget::paintEvent(QPaintEvent * e)
{
  (void)e;
  redraw();
}

/*void GLLUTWidget::saveImage()
{
  rgbImage temp;
  if (rb!=0) {
    int idx=rb->curRead();
    FrameData * frame = rb->getPointer(idx);
    temp.copy(rgbImage(frame->video));
  }
  if (temp.getWidth() > 1 && temp.getHeight() > 1) {
    QFileDialog dialog(this,
                       "Export image to file...",
                       "",
                       "PPM (*.ppm)");
    dialog.setConfirmOverwrite(true);
    dialog.setDefaultSuffix("ppm");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
      if (dialog.selectedFiles().size() > 0) {
        QString filename=dialog.selectedFiles().at(0);
        if (temp.save(filename.toStdString())) {
          QMessageBox::information(this, "Success","PPM was saved successfully.");
        } else {
          QMessageBox::warning(this, "Error","Unknown error while saving ppm.");
        }
      }
    }
  }
}*/

void GLLUTWidget::clearSampler() {
  for (int i=0;i<(int)slices.size();i++) {
    slices[i]->sampler->surface.fillBlack();
    slices[i]->sampler_update_pending=true;
  }
  redraw();
}
