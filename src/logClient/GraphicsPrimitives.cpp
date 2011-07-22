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
  \file    GraphicsPrimitives.cpp
  \brief   C++ Implementation: Robot, SoccerView
  \author  Joydeep Biswas (C) 2009
  \edit    Ulfert Nehmiz (LogPlayer included) 2009
*/
//========================================================================

#include "GraphicsPrimitives.h"
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QWheelEvent>
#include "math.h"

Robot::Robot()
{
  Robot ( 0,0,0,teamUnknown,0,0,0 );
}

Robot::Robot ( double _x, double _y, double _orientation, int _teamID, int _id, int _key, double _conf )
{
  conf = _conf;
  key = _key;
  setVisible ( true );
  setZValue ( 1 );
  x = _x;
  y = _y;
  orientation = _orientation;
  teamID = _teamID;
  id = _id;

  robotOutline.moveTo ( 90,0 );
  robotOutline.arcTo ( -90,-90,180,180,0,270 );
  robotOutline.closeSubpath();
  robotOutlineCircle.addEllipse ( -90,-90,180,180 );

  if ( id!=NA )
    robotLabel = QString ( QByteArray ( ( char * ) &id, 4 ).toHex() ).mid ( 1,1 ).toUpper();
  else
    robotLabel = "?";

  drawFont = QFont ( "Courier",80,2,false );
  robotID.addText ( -25,25,QFont ( "Courier",80,2,false ),robotLabel );
  switch ( teamID )
  {
    case teamBlue:
    {
      brush = new QBrush ( QColor ( 0x41,0x7e,0xff,255 ),Qt::SolidPattern );
      pen = new QPen ( QColor ( 0x12,0x3b,0xa0,255 ) );
      break;
    }
    case teamYellow:
    {
      brush = new QBrush ( QColor ( 0xff,0xf3,0x3e,255 ),Qt::SolidPattern );
      pen = new QPen ( QColor ( 0xcc,0x9d,0x00,255 ) );
      break;
    }
    default:
    {
      brush = new QBrush ( QColor ( 150,150,150,255 ),Qt::SolidPattern );
      pen = new QPen ( QColor ( 70,70,70,255 ) );
      break;
    }
  }
  pen->setWidth ( 6 );
  idPen = new QPen ( Qt::black );
  idPen->setWidth ( 0 );
  confPen = new QPen ( Qt::white );
  confPen->setWidth ( 1 );
  tStamp = GetTimeUSec();
}

Robot::~Robot()
{
  delete brush;
  delete pen;
  delete idPen;
}

QRectF Robot::boundingRect() const
{
  return QRectF ( -95+x,-95-y,190,190 );
}

void Robot::paint ( QPainter *painter, const QStyleOptionGraphicsItem* , QWidget* )
{
  if ( conf==0.0 )
    return;
  painter->translate ( x,-y );
  painter->setPen ( *pen );
  painter->setBrush ( *brush );
  if ( fabs ( orientation ) <360 )
  {
    painter->rotate ( -45-orientation );
    painter->drawPath ( robotOutline );
    painter->rotate ( 45+orientation );
  }
  else
    painter->drawPath ( robotOutlineCircle );

  painter->setPen ( *idPen );
  painter->setBrush ( Qt::black );
  painter->setFont ( drawFont );
  //painter->drawPath(robotID);
  painter->drawText ( -45,-60,1000,1000,0,robotLabel );

  painter->setPen ( Qt::NoPen );
  painter->setBrush ( *brush );
  painter->drawRect ( -90,-130, ( int ) ( ( ( double ) 180 ) *conf ),30 );
  painter->setPen ( *pen );
  painter->setBrush ( QBrush ( Qt::white, Qt::NoBrush ) );
  painter->drawRect ( -90,-130,180,30 );
}

QPainterPath Robot::shape() const
{
  QPainterPath path;
  path.addEllipse ( -10, -10, 20, 20 );
  return path;
}

void Robot::SetPose ( double _x, double _y, double _orientation, double _conf )
{
  x = _x;
  y = _y;
  orientation = _orientation;
  conf = _conf;
  tStamp = GetTimeUSec();
}

SoccerView::SoccerView(QMutex* _drawMutex)
{
  drawMutex = _drawMutex;
  shutdownSoccerView = false;
  glWidget = new QGLWidget ( QGLFormat ( QGL::DoubleBuffer ) );
  setViewport ( glWidget );
  LoadFieldGeometry();
  scene = new QGraphicsScene ( this );
  setScene ( scene );
  this->setViewportUpdateMode ( QGraphicsView::NoViewportUpdate );
  scene->setBackgroundBrush ( QBrush ( QColor ( 0,0x91,0x19,255 ),Qt::SolidPattern ) );
  scene->setSceneRect ( -3700,-2700,7400,5400 );
  ConstructField();
  fieldBrush = new QBrush ( Qt::NoBrush );
  fieldLinePen = new QPen();
  fieldLinePen->setColor ( Qt::white );
  fieldLinePen->setWidth ( 10 );
  fieldLinePen->setJoinStyle ( Qt::MiterJoin );
  fieldItem = scene->addPath ( *field,*fieldLinePen,*fieldBrush );
  fieldItem->setZValue(0);
  drawScale = 0.15;
  //this->setRenderHint ( QPainter::Antialiasing, true );
  this->setRenderHint ( QPainter::HighQualityAntialiasing, true );
  this->setDragMode ( QGraphicsView::ScrollHandDrag );
  this->setGeometry ( 100,100,1036,756 );
  scalingRequested = true;

  //LogPlayer data
  playLogfile = new QPushButton(this);
  playLogfile->setObjectName(QString::fromUtf8("play_logfile_button"));
  playLogfile->setCheckable(false);
  playLogfile->setText("Play Logfile");
  playLogfile->setGeometry(10, 10, 150, 30);

  //Timer for casual redraw
  startTimer ( 1000 );
}

SoccerView::~SoccerView()
{
  shutdownSoccerView = true;
}

void SoccerView::initView()
{
  drawScale = pow ( 2.0, (-120) / 540.0 );
  scalingRequested = true;
}

void SoccerView::updateView()
{
  static float lastScale = 0;
  if ( shutdownSoccerView )
  {
    return;
  }
  if ( scalingRequested )
  {
    qreal factor = matrix().scale ( drawScale, drawScale ).mapRect ( QRectF ( 0, 0, 1, 1 ) ).width();
    if ( factor > 0.07 && factor < 100.0 )
      scale ( drawScale, drawScale );
    scalingRequested = false;
  }
  //causes segfaults, update-queue is evil
  //*** glibc detected *** PATH/ssl-vision-read-only/bin/graphicalClient: malloc(): memory corruption (fast): 0xb15682f2 ***

  //  Program received signal SIGSEGV, Segmentation fault.
  //  [Switching to Thread 0xb494ab90 (LWP 13615)]
  //  0xb766db63 in QRegion::operator+= () from /usr/lib/libQtGui.so.4

  this->viewport()->update();
}

void SoccerView::timerEvent ( QTimerEvent *event )
{
  if ( shutdownSoccerView )
  {
    killTimer ( event->timerId() );
    return;
  }
  drawMutex->lock();
  this->updateView();
  drawMutex->unlock();
}

void SoccerView::ConstructField()
{
  //scene->removeItem(fieldItem);
  field = new QPainterPath();

  field->moveTo ( 0,-field_width/2 );
  field->lineTo ( 0,field_width/2 );

  field->addEllipse ( -center_circle_radius,-center_circle_radius,
                      2*center_circle_radius,2*center_circle_radius );

  field->moveTo ( field_length/2,-field_width/2 );
  field->lineTo ( field_length/2,field_width/2 );

  field->moveTo ( -field_length/2,-field_width/2 );
  field->lineTo ( -field_length/2,field_width/2 );

  field->moveTo ( -field_length/2,-field_width/2 );
  field->lineTo ( field_length/2,-field_width/2 );

  field->moveTo ( -field_length/2,field_width/2 );
  field->lineTo ( field_length/2,field_width/2 );

  field->moveTo ( field_length/2,goal_width/2 );
  field->lineTo ( ( field_length/2+goal_depth ),goal_width/2 );
  field->lineTo ( ( field_length/2+goal_depth ),-goal_width/2 );
  field->lineTo ( field_length/2,-goal_width/2 );
  field->moveTo ( ( field_length/2-defense_radius ),defense_stretch/2 );
  field->lineTo ( ( field_length/2-defense_radius ),-defense_stretch/2 );
  field->moveTo ( ( field_length/2-defense_radius ),defense_stretch/2 );
  field->arcTo ( ( field_length/2-defense_radius ),- ( defense_radius-defense_stretch/2 ),defense_radius*2,defense_radius*2,180,90 );
  field->moveTo ( ( field_length/2-defense_radius ),-defense_stretch/2 );
  field->arcTo ( ( field_length/2-defense_radius ),- ( defense_radius+defense_stretch/2 ),defense_radius*2,defense_radius*2,180,-90 );

  field->moveTo ( -field_length/2,goal_width/2 );
  field->lineTo ( - ( field_length/2+goal_depth ),goal_width/2 );
  field->lineTo ( - ( field_length/2+goal_depth ),-goal_width/2 );
  field->lineTo ( -field_length/2,-goal_width/2 );
  field->moveTo ( - ( field_length/2-defense_radius ),defense_stretch/2 );
  field->lineTo ( - ( field_length/2-defense_radius ),-defense_stretch/2 );
  field->moveTo ( - ( field_length/2-defense_radius ),defense_stretch/2 );
  field->arcTo ( - ( field_length/2+defense_radius ),- ( defense_radius-defense_stretch/2 ),defense_radius*2,defense_radius*2,0,-90 );
  field->moveTo ( - ( field_length/2-defense_radius ),-defense_stretch/2 );
  field->arcTo ( - ( field_length/2+defense_radius ),- ( defense_radius+defense_stretch/2 ),defense_radius*2,defense_radius*2,0,90 );
}

void SoccerView::scaleView ( qreal scaleFactor )
{
  qreal factor = matrix().scale ( scaleFactor, scaleFactor ).mapRect ( QRectF ( 0, 0, 1, 1 ) ).width();
  if ( factor > 0.07 && factor < 100.0 )
    drawScale = scaleFactor;
}

void SoccerView::wheelEvent ( QWheelEvent *event )
{
  scrollMutex.lock();
  drawScale = pow ( 2.0, event->delta() / 540.0 );
  scalingRequested = true;
  drawMutex->lock();
  this->updateView();
  drawMutex->unlock();
  event->ignore();
  scrollMutex.unlock();
}

void SoccerView::AddRobot ( Robot *robot )
{
  robots.append ( robot );
  scene->addItem ( robot );
}

void SoccerView::UpdateRobots ( SSL_DetectionFrame &detection )
{
  int robots_blue_n =  detection.robots_blue_size();
  int robots_yellow_n =  detection.robots_yellow_size();
  int i,j,yellowj=0,bluej=0;
  int team=teamBlue;
  SSL_DetectionRobot robot;
  for ( i = 0; i < robots_blue_n+robots_yellow_n; i++ )
  {
    if ( i<robots_blue_n )
    {
      robot = detection.robots_blue ( i );
      team = teamBlue;
      j=bluej;
    }
    else
    {
      robot = detection.robots_yellow ( i-robots_blue_n );
      team = teamYellow;
      j=yellowj;
    }

    double x,y,orientation,conf =robot.confidence();
    int id=NA, n=0;
    if ( robot.has_robot_id() )
      id = robot.robot_id();
    else
      id = NA;
    x = robot.x();
    y = robot.y();
    if ( robot.has_orientation() )
      orientation = robot.orientation() *180.0/M_PI;
    else
      orientation = NAOrientation;

    //seek to the next robot of the same camera and team colour
    while ( j<robots.size() && ( robots[j]->key!=detection.camera_id() || robots[j]->teamID!=team ) )
      j++;

    if ( j+1>robots.size() )
      AddRobot ( new Robot ( x,y,orientation,team,id,detection.camera_id(),conf ) );

    robots[j]->SetPose ( x,y,orientation,conf );
    QString label;

    if ( id!=NA )
      label.setNum ( id,16 );
    else
      label = "?";
    label = label.toUpper();
    if ( label!=robots[j]->robotLabel )
      robots[j]->robotLabel = label;

    j++;

    if ( i<robots_blue_n )
      bluej=j;
    else
      yellowj=j;
  }
  for ( j=bluej;j<robots.size();j++ )
  {
    if ( robots[j]->key==detection.camera_id() && robots[j]->teamID==teamBlue )
      robots[j]->conf=0.0;
  }
  for ( j=yellowj;j<robots.size();j++ )
  {
    if ( robots[j]->key==detection.camera_id() && robots[j]->teamID==teamYellow )
      robots[j]->conf=0.0;
  }
  return;
}

int SoccerView::UpdateBalls ( QVector<QPointF> &_balls, int cameraID )
{
  QVector<QGraphicsEllipseItem*> tmp;
  while(cameraID+1>ballItems.size())
    ballItems.append(tmp);

  if ( ballItems[cameraID].size() < _balls.size() ){
    //need to allocate some space for the new balls
    QPen pen ( QColor ( 0xcd,0x59,0x00,0xff ) );
    pen.setWidth ( 4 );
    QBrush brush ( QColor ( 0xff,0x81,0x00,0xff ),Qt::SolidPattern );
    while(_balls.size()>ballItems[cameraID].size()){
      ballItems[cameraID].append ( scene->addEllipse ( 0,0,42,42,pen,brush ) );
      ballItems[cameraID][ballItems[cameraID].size()-1]->setZValue(2);
    }
  }
  else if ( ballItems[cameraID].size() >_balls.size() ){
    //need to delete some balls
    while(ballItems[cameraID].size()>_balls.size()){
      scene->removeItem ( ballItems[cameraID][0] );
      ballItems[cameraID].remove(0);
    }
  }

  for ( int i=0;i<_balls.size();i++ ){
    //Let's update the ball positions now
    ballItems[cameraID][i]->setPos ( _balls[i].x()-21,-_balls[i].y()-21 );
  }

  int balls = ballItems[cameraID].size();
  return balls;
}

void SoccerView::LoadFieldGeometry()
{
  this->line_width = FieldConstantsRoboCup2009::line_width;
  this->field_length = FieldConstantsRoboCup2009::field_length;
  this->field_width = FieldConstantsRoboCup2009::field_width;
  this->boundary_width = FieldConstantsRoboCup2009::boundary_width;
  this->referee_width = FieldConstantsRoboCup2009::referee_width;
  this->goal_width = FieldConstantsRoboCup2009::goal_width;
  this->goal_depth = FieldConstantsRoboCup2009::goal_depth;
  this->goal_wall_width = FieldConstantsRoboCup2009::goal_wall_width;
  this->center_circle_radius = FieldConstantsRoboCup2009::center_circle_radius;
  this->defense_radius = FieldConstantsRoboCup2009::defense_radius;
  this->defense_stretch = FieldConstantsRoboCup2009::defense_stretch;
  this->free_kick_from_defense_dist = FieldConstantsRoboCup2009::free_kick_from_defense_dist;
  this->penalty_spot_from_field_line_dist = FieldConstantsRoboCup2009::penalty_spot_from_field_line_dist;
  this->penalty_line_from_spot_dist = FieldConstantsRoboCup2009::penalty_line_from_spot_dist;
}

void SoccerView::LoadFieldGeometry ( SSL_GeometryFieldSize &fieldSize )
{
  this->line_width = fieldSize.line_width();
  this->field_length = fieldSize.field_length();
  this->field_width = fieldSize.field_width();
  this->boundary_width = fieldSize.boundary_width();
  this->referee_width = fieldSize.referee_width();
  this->goal_width = fieldSize.goal_width();
  this->goal_depth = fieldSize.goal_depth();
  this->goal_wall_width = fieldSize.goal_wall_width();
  this->center_circle_radius = fieldSize.center_circle_radius();
  this->defense_radius = fieldSize.defense_radius();
  this->defense_stretch = fieldSize.defense_stretch();
  this->free_kick_from_defense_dist = fieldSize.free_kick_from_defense_dist();
  this->penalty_spot_from_field_line_dist = fieldSize.penalty_spot_from_field_line_dist();
  this->penalty_line_from_spot_dist = fieldSize.penalty_line_from_spot_dist();

  scene->removeItem ( fieldItem );
  ConstructField();
  fieldItem = scene->addPath ( *field,*fieldLinePen,*fieldBrush );
}

