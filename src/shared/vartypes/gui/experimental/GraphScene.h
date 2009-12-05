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
  \file    GraphScene.h
  \brief   C++ Interface: GraphScene
  \author  Stefan Zickler, (C) 2008
*/

#ifndef GRAPHDRAWWIDGET_H_
#define GRAPHDRAWWIDGET_H_
#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "TimeLine.h"
#include "TimeControl.h"
#include "VarTypes.h"
class GraphScene : public QGraphicsScene
{
  Q_OBJECT
  QList<TimeLine *> lines;
protected:
  TimeControl * control;
  bool draw_smooth;
  bool shade_area;
  QPainterPath makePath(TimeLine * l, int minidx, int maxidx, bool zero_edges);
public:
  void setDrawSmooth(bool val) {
    draw_smooth=val;
  }
  
  void setShadeArea(bool val) {
    shade_area=val;
  }
  
  GraphScene(QObject * parent=0);
  void setTimeControl(TimeControl * tcontrol) {
    control=tcontrol;
    for (int i=0;i<lines.size();i++) {
      lines.at(i)->setTimePointer(control->getTimePointer());
    }
  }
  virtual ~GraphScene();
  void addVariable(VarType * tl);
  //let's use direct drawing instead of storing items in the scene.
  void drawBackground ( QPainter * painter, const QRectF & rect ); 
  void drawForeground ( QPainter * painter, const QRectF & rect );

  void updateArea();
  
  void mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent );
  
};

#endif /*GRAPHDRAWWIDGET_H_*/
