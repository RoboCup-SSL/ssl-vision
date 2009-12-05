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
  \file    GraphScene.cpp
  \brief   C++ Implementation: GraphScene
  \author  Stefan Zickler, (C) 2008
*/
#include "GraphScene.h"

GraphScene::GraphScene( QObject * parent) : QGraphicsScene(parent)
{
	setDrawSmooth(true);
	setShadeArea(true);
}

GraphScene::~GraphScene()
{
}



void GraphScene::updateArea() {
	ValueRange limit_value(0,1.0);

	
	for (int i=0;i < lines.size();i++) {
		if (i==0) {
			limit_value=lines.at(i)->getValueRange();
		} else {
			ValueRange tmp_value=lines.at(i)->getValueRange();
			if (tmp_value.getMin() < limit_value.getMin()) limit_value.setMin(tmp_value.getMin());
			if (tmp_value.getMax() > limit_value.getMax()) limit_value.setMax(tmp_value.getMax());
		}		
	}
	setSceneRect(control->getFullRange()->getMin(),limit_value.getMin(),control->getFullRange()->getLength(),limit_value.getLength());
	//setSceneRect(control->getFullRange()->getMin(),-10,control->getFullRange()->getLength(),20);
	qDebug("scene rect: %f %f %f %f\n",(double)control->getFullRange()->getMin(),(double)limit_value.getMin(),(double)control->getFullRange()->getLength(),(double)limit_value.getLength());
}

void GraphScene::drawBackground ( QPainter * painter, const QRectF & rect ) {
	//clear the background in dark grey
	
	
	painter->setRenderHints(QPainter::Antialiasing);
	painter->save();
		QBrush bg(QColor(20,20,20));
		painter->setBrush(bg);
		painter->drawRect(rect);
	painter->restore();
	
	painter->save();
		QPen linepen(QColor(200,200,200), 1, Qt::SolidLine);
		linepen.setCosmetic(true);
		painter->setPen(linepen);
		painter->drawLine(control->getFullRange()->getMin(),0,control->getFullRange()->getMax(),0);
	painter->restore();
	//draw slightly visible second time-marks depending on range
	
	//draw textual time-labels also depending on range
	
	
	
	
}

void GraphScene::addVariable(VarType * tl) {
	if (tl->getType()==DT_TIMELINE) {
		((TimeLine *)tl)->setTimePointer(control->getTimePointer());
		lines.append((TimeLine *)tl);
	} else {
		qDebug("cannot append non-timeline datatype to graphscene");
	}
}
 
void GraphScene::drawForeground ( QPainter * painter, const QRectF & rect ) {
	//draw all graphs 
	for (int i=0;i < lines.size();i++) {
		TimeLine * l=lines.at(i);
		int minidx=l->findIdx(control->getVisibleRange()->getMin());
		if (minidx > 0) minidx--;
		int maxidx=l->findIdx(control->getVisibleRange()->getMax());
		if (maxidx < (l->size()-1)) maxidx++;
		
		if (minidx >= 0 && maxidx >= 0) {
			QPainterPath path;
	
			painter->save();
			
			 	painter->setRenderHints(QPainter::Antialiasing);
			 	QColor c(0,192,255);
			 	if (shade_area) {
			 		path = makePath(l,minidx,maxidx, true);
			 		c.setAlpha(128);
			 		painter->setBrush( QBrush(c) );
					painter->setPen ( Qt::NoPen );
					painter->drawPath(path);
			 	}
			 	c.setAlpha(255);
			 	path = makePath(l,minidx,maxidx, false);
			 	QPen graphpen(c, 2, Qt::SolidLine);
			 	graphpen.setCosmetic(true);
			 	painter->setBrush( Qt::NoBrush );
			 	painter->setPen (graphpen);
			 	painter->drawPath(path);
		 	painter->restore();
		}
	}
	
	//draw pointer
	painter->save();
		//painter->setCompositionMode(QPainter::CompositionMode_Overlay);
		QPen ppen (QColor(255,255,0,128),2,Qt::SolidLine);
		ppen.setCosmetic(true);
		painter->setPen (ppen);

		painter->drawLine(QPointF(control->getTimePointer()->get(),rect.y()),QPointF(control->getTimePointer()->get(),rect.height()));
	painter->restore();
	
	painter->save();
		painter->resetTransform();
		QPen tpen (QColor(255,255,255),1,Qt::SolidLine);
		tpen.setCosmetic(true);
		painter->setPen (tpen);
		
		painter->drawText(QPointF(0,10),"test");
	painter->restore();
	//draw pointer if needed
}

QPainterPath GraphScene::makePath(TimeLine * l, int minidx, int maxidx, bool zero_edges) {
		QPainterPath path;
	 	QPointF p1;
	 	QPointF p2;
		for (int j=minidx;j<=maxidx;j++) {			 		
			 		p2.setX(((TimeIndex*)l->at(j))->getTime());
		 			p2.setY(l->at(j)->getData()->getValue());
			 		if (zero_edges==true && j==minidx) {
			 			path.moveTo(QPointF(p2.x(),0));
			 		}
		    	if (draw_smooth==false) {
		    		if (j==minidx && zero_edges ==false) {
		    			path.moveTo(p2);
		    		} else {
		    			path.lineTo(p2);
		    		}			    		
		    	} else {
		    		if (j==minidx) {
		    			if (zero_edges) {
		    				path.lineTo(p2);
		    			} else {
		    				path.moveTo(p2);
		    			}
		  			} else {
		    			double distance = p2.x() - p1.x();
		    			path.cubicTo(p1.x() + distance / 2, p1.y(),
							p1.x() + distance / 2, p2.y(),
							p2.x(), p2.y());    		
		    		}
		    	}
			 		if (zero_edges==true && j==maxidx) {
			 			path.lineTo(QPointF(p2.x(),0));
			 		}
		    	
			 		p1=p2;
				}
		
		return path;
}

void GraphScene::mouseMoveEvent ( QGraphicsSceneMouseEvent * e ) {
	if (e->buttons() & Qt::LeftButton) {
		control->getTimePointer()->set(e->scenePos().x());
		update();
	}
	
	//control->hasChanged();
}


