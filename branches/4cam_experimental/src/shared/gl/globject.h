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
  \file    globject.h
  \brief   C++ Interface: GLObject
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef GLOBJECT_H
#define GLOBJECT_H
#include "GL/gl.h"
#include "geometry.h"

/*!
  \class GLObject
  \brief A baseclass for all GL scene-objects
         it carries full 3d pose information (position, rotation)
         and scaling in object-aligned coordinate space
  \author  Stefan Zickler, (C) 2008
*/
class GLObject{
public:
    GLObject();
    virtual ~GLObject();
    void reset();
    vector3d scale;
    vector3d pos;
    quat rot;
    virtual void apply();
protected:
    virtual void preRender();
    virtual void render();
    virtual void postRender();
};

/*!
  \class GLMaterial
  \brief A class to simplify the use of OpenGL materials
  \author  Stefan Zickler, (C) 2008
*/
class GLMaterial{
protected:
  GLfloat mat_ambient[4];// = {0.3f,0.3f,0.3f,0.5f};
  GLfloat mat_diffuse[4];// = {0.25, 0.25, 0.25, 0.5};
  GLfloat mat_specular[4];// = {1.0, 1.0, 1.0, 1.0};
  GLfloat mat_emission[4];// = {0.0, 0.0, 0.0, 1.0};
  GLfloat mat_shininess[1];// = {50.0};
public:

  void set(const GLMaterial & val) {
    val.getAmbient(mat_ambient);
    val.getDiffuse(mat_diffuse);
    val.getSpecular(mat_specular);
    val.getEmission(mat_emission);
    val.getShininess(mat_shininess);
  };

  void operator = (const GLMaterial & val) {
    set(val);
  };

  void setRGB(GLfloat r, GLfloat g, GLfloat b) {
    setAmbient(r,g,b,1.0);
    setDiffuse(r,g,b,1.0);
    setSpecular(1.0,1.0,1.0,0.3);
    setEmission(0.0,0.0,0.0,0.0);
    setShininess(50.0);
  }
  GLMaterial(GLfloat r, GLfloat g, GLfloat b) {
    setRGB(r,g,b);
  };
  GLMaterial() {
    setRGB(0.7,0.7,0.7);
  }
  ~GLMaterial() {};

  void setAlpha(double ambient, double diffuse, double specular, double emission) {
    mat_ambient[3]=ambient;
    mat_diffuse[3]=diffuse;
    mat_specular[3]=specular;
    mat_emission[3]=emission;
  };

  void setAmbient( const GLfloat val[4]) {
    mat_ambient[0]=val[0];
    mat_ambient[1]=val[1];
    mat_ambient[2]=val[2];
    mat_ambient[3]=val[3];
  };
  void setDiffuse(const GLfloat val[4]) {
    mat_diffuse[0]=val[0];
    mat_diffuse[1]=val[1];
    mat_diffuse[2]=val[2];
    mat_diffuse[3]=val[3];
  };
  void setSpecular(const GLfloat val[4]) {
    mat_specular[0]=val[0];
    mat_specular[1]=val[1];
    mat_specular[2]=val[2];
    mat_specular[3]=val[3];
  };  
  void setEmission(const GLfloat val[4]) {
    mat_emission[0]=val[0];
    mat_emission[1]=val[1];
    mat_emission[2]=val[2];
    mat_emission[3]=val[3];
  };
  void setShininess(const GLfloat val[1]) {
    mat_shininess[0]=val[0];
  };

  
  void setAmbient(const GLfloat & r,const GLfloat & g,const GLfloat & b,const GLfloat & a=1.0) {
    mat_ambient[0]=r;
    mat_ambient[1]=g;
    mat_ambient[2]=b;
    mat_ambient[3]=a;
  };
  void setDiffuse(const GLfloat & r,const GLfloat & g,const GLfloat & b,const GLfloat & a=1.0) {
    mat_diffuse[0]=r;
    mat_diffuse[1]=g;
    mat_diffuse[2]=b;
    mat_diffuse[3]=a;
  };
  void setSpecular(const GLfloat & r,const GLfloat & g,const GLfloat & b,const GLfloat & a=1.0) {
    mat_specular[0]=r;
    mat_specular[1]=g;
    mat_specular[2]=b;
    mat_specular[3]=a;
  };  
  void setEmission(const GLfloat & r,const GLfloat & g,const GLfloat & b,const GLfloat & a=1.0) {
    mat_emission[0]=r;
    mat_emission[1]=g;
    mat_emission[2]=b;
    mat_emission[3]=a;
  };
  void setShininess(const GLfloat & val) {
    mat_shininess[0]=val;
  };
  
  void getAmbient(GLfloat val[4]) const {
    val[0]=mat_ambient[0];
    val[1]=mat_ambient[1];
    val[2]=mat_ambient[2];
    val[3]=mat_ambient[3];
  };
  void getDiffuse(GLfloat val[4]) const { 
    val[0]=mat_diffuse[0];
    val[1]=mat_diffuse[1];
    val[2]=mat_diffuse[2];
    val[3]=mat_diffuse[3];
  };
  void getSpecular(GLfloat val[4]) const {
    val[0]=mat_specular[0];
    val[1]=mat_specular[1];
    val[2]=mat_specular[2];
    val[3]=mat_specular[3];
  };
  void getEmission(GLfloat val[4]) const {
    val[0]=mat_emission[0];
    val[1]=mat_emission[1];
    val[2]=mat_emission[2];
    val[3]=mat_emission[3];
  };
  void getShininess(GLfloat val[1]) const {
    val[0]=mat_shininess[0];
  };

  void apply() {
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,mat_emission);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,mat_shininess);
  }

};

#endif
