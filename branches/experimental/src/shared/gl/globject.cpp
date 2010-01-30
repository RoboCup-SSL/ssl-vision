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
  \file    globject.cpp
  \brief   C++ Implementation: GLObject
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================
#include "globject.h"

GLObject::GLObject() {
  reset();
}

GLObject::~GLObject() {
}

void GLObject::reset() {
  scale.set(1.0,1.0,1.0);
  pos.set(0.0,0.0,0.0);
  rot.clear();
}

void GLObject::preRender() {
  double m[16];
  rot.getMatrix(m);
  glTranslated(pos.x,pos.y,pos.z);
  glMultMatrixd(m);
 glScaled(scale.x,scale.y,scale.z);
}

void GLObject::postRender() {

}

void GLObject::render() {

}

void GLObject::apply() {
  glPushMatrix();
  preRender();
  render();
  postRender();
  glPopMatrix();
}

