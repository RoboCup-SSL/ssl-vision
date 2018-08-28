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
  \file    zoom.h
  \brief   C++ Interface: Zoom
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef ZOOM_H_
#define ZOOM_H_

#include "util.h"
#include "pixelloc.h"
#define DEFAULT_ZOOM_INCREMENT 1.25
#define DEFAULT_PAN_DISTANCE 0.01
#define MIN_ZOOM 0.1
#define MAX_ZOOM 50.0
#define MAX_PAN 2.0
#define MIN_PAN -2.0

/*!
  \class  Zoom
  \brief  A class for computation of 2D zoom + pan operations
  \author Stefan Zickler, (C) 2008
*/
class Zoom {
  protected:
    double IMAGE_WIDTH;
    double IMAGE_HEIGHT;
    double SURFACE_WIDTH;
    double SURFACE_HEIGHT;
    bool CENTER_ORIGIN;
    double flipX;
    double flipY;
    double img_center_x;
    double img_center_y;
    double zoomFactor;
    double zoomCenterX,zoomCenterY;
  public:
    double getZoom() {
      return zoomFactor;
    }
    double getPanX() {
      return zoomCenterX;
    }
    double getPanY() {
      return zoomCenterY;
    }
    void reset() {
      flipX=1.0;
      flipY=1.0;
      zoomFactor=1.0;
      zoomCenterX=0.0;
      zoomCenterY=0.0;
    }
    void setup ( int img_width, int img_height, int surface_width, int surface_height, bool center_origin=false ) {
      //if center_origin is true then it is assumed that pixel coordinate 0,0 is in the
      //center of the viewport (after loading glLoadIdentity() in ortho mode this is usually the case)
      //if it's false then its assumes that the origin is in the corner of the viewport
      CENTER_ORIGIN=center_origin;
      IMAGE_WIDTH=img_width;
      IMAGE_HEIGHT=img_height;
      img_center_x=IMAGE_WIDTH/2.0;
      img_center_y=IMAGE_HEIGHT/2.0;
      SURFACE_WIDTH=surface_width;
      SURFACE_HEIGHT=surface_height;
    }

    void setCustomImageCenter ( double x, double y ) {
      img_center_x=x;
      img_center_y=y;
    }

    void setDefaultImageCenter() {
      img_center_x=IMAGE_WIDTH/2.0;
      img_center_y=IMAGE_HEIGHT/2.0;
    }

    bool getFlipX() {
      return ( flipX < 0 );
    }
    bool getFlipY() {
      return ( flipY < 0 );
    }
    double getFlipXval() {
      return flipX;
    }
    double getFlipYval() {
      return flipY;
    }
    void setFlipX ( bool val ) {
      if ( val ) {
        flipX= ( -1.0 );
      } else {
        flipX=1.0;
      }

    }
    void setFlipY ( bool val ) {
      if ( val ) {
        flipY= ( -1.0 );
      } else {
        flipY=1.0;
      }
    }

    void zoomToFit() {
      setZoom ( min ( ( double ) SURFACE_WIDTH/ ( double ) IMAGE_WIDTH, ( double ) SURFACE_HEIGHT/ ( double ) IMAGE_HEIGHT ) );
    }

    void setZoom ( double val ) {
      if ( val < MIN_ZOOM ) val=MIN_ZOOM;
      if ( val > MAX_ZOOM ) val=MAX_ZOOM;
      zoomFactor=val;
    }

    void setPan ( double valX, double valY ) {
      if ( valX < MIN_PAN ) valX=MIN_PAN;
      if ( valX > MAX_PAN ) valX=MAX_PAN;
      if ( valY < MIN_PAN ) valY=MIN_PAN;
      if ( valY > MAX_PAN ) valY=MAX_PAN;
      zoomCenterX=valX;
      zoomCenterY=valY;
    }
    void panLeft ( double distance=DEFAULT_PAN_DISTANCE ) {
      setPan ( getPanX()- ( distance*flipX ),getPanY() );
    }
    void panRight ( double distance=DEFAULT_PAN_DISTANCE ) {
      setPan ( getPanX() + ( distance*flipX ),getPanY() );
    }
    void panUp ( double distance=DEFAULT_PAN_DISTANCE ) {
      setPan ( getPanX(),getPanY()- ( distance*flipY ) );
    }
    void panDown ( double distance=DEFAULT_PAN_DISTANCE ) {
      setPan ( getPanX(),getPanY() + ( distance*flipY ) );
    }
    void zoomIn ( double factor=DEFAULT_ZOOM_INCREMENT ) {
      setZoom ( getZoom() *factor );
    }
    void zoomOut ( double factor=DEFAULT_ZOOM_INCREMENT ) {
      setZoom ( getZoom() /factor );
    }
    Zoom() {
      CENTER_ORIGIN=false;
      setup ( 320,240,320,240 );
      reset();
    }

    virtual ~Zoom() {

    }

    QTransform getQTransform ( bool center_origin ) {

      if ( center_origin ) {
        QTransform t ( zoomFactor * flipX, 0.0,0.0,zoomFactor * flipY,
                       ( ( - img_center_x )   + ( zoomCenterX * SURFACE_WIDTH ) ) * ( zoomFactor * flipX ) ,
                       ( ( - img_center_y )   + ( zoomCenterY * SURFACE_HEIGHT ) ) * ( zoomFactor * flipY ) );

        return t;
      } else {
              QTransform t ( zoomFactor * flipX, 0.0,0.0,zoomFactor * flipY,
                       ( ( - img_center_x )   + ( zoomCenterX * SURFACE_WIDTH ) ) * ( zoomFactor * flipX ) + SURFACE_WIDTH / 2.0,
                       ( ( - img_center_y )   + ( zoomCenterY * SURFACE_HEIGHT ) ) * ( zoomFactor * flipY ) + SURFACE_HEIGHT / 2.0 );

        return t;
      }

    }

    pixelloc zoom ( int x, int y ) {
      pixelloc result;
      double tx,ty;
      tx= x;
      ty= y;
      if ( CENTER_ORIGIN ) {
        result.x= ( int ) ( ( tx - ( img_center_x )   + ( zoomCenterX * SURFACE_WIDTH ) ) * ( zoomFactor * flipX ) );
        result.y= ( int ) ( ( ty - ( img_center_y )  + ( zoomCenterY * SURFACE_HEIGHT ) ) * ( zoomFactor * flipY ) );
      } else {
        result.x= ( int ) ( ( ( tx - ( img_center_x )   + ( zoomCenterX * SURFACE_WIDTH ) ) * ( zoomFactor * flipX ) ) + SURFACE_WIDTH/2.0 ) ;
        result.y= ( int ) ( ( ( ty - ( img_center_y )  + ( zoomCenterY * SURFACE_HEIGHT ) ) * ( zoomFactor * flipY ) ) + SURFACE_HEIGHT/2.0 ) ;
      }

      return ( result );
    }

    pixelloc invZoom ( int x, int y , bool disable_center_origin=false ) {
      pixelloc result;
      double tx= ( double ) x;
      double ty= ( double ) y;
      if ( CENTER_ORIGIN==false || disable_center_origin ) {
        tx-= ( double ) SURFACE_WIDTH/2.0;
        ty-= ( double ) SURFACE_HEIGHT/2.0;
      }
      result.x = ( int ) ( ( ( ( double ) tx / ( zoomFactor * flipX ) ) - ( zoomCenterX * SURFACE_WIDTH ) + ( img_center_x ) ) - ( 0.0 ) );
      result.y = ( int ) ( ( ( ( double ) ty / ( zoomFactor * flipY ) ) - ( zoomCenterY * SURFACE_HEIGHT ) + ( img_center_y ) ) - ( getFlipY() ? 0.0 : 0.5 ) );
      return result;
    }

};

#endif /*ZOOM_H_*/
