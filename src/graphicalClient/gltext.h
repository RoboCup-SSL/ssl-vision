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
\file    gltext.h
\brief   C++ Interface: GLText
\author  Joydeep Biswas (C) 2011
*/
//========================================================================

#include <QVector>
#include <QtGui>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <float.h>
#include "geometry.h"
#include "util.h"

#ifndef GL_TEXT_H
#define GL_TEXT_H

class GLText{
  struct Glyph{
    bool compiled;
    GLuint displayListID;
    double width;
    double height;
    double ascent;
    double descent;
    Glyph(){compiled=false;}
  };
  
  QVector<Glyph> glyphs;
  
  double characterSpacing;
  static const bool debugTesselation;
  QFont font;
  
public:
  
  typedef enum{
    LeftAligned,
    RightAligned,
    CenterAligned
  }HAlignOptions;
  
  typedef enum{
    TopAligned,
    BottomAligned,
    MedianAligned,
    MiddleAligned
  }VAlignOptions;
  
  GLText(QFont font = QFont());
  ~GLText();
  void drawString(vector2d loc, double angle, double size, const char* str, GLText::HAlignOptions hAlign=LeftAligned, GLText::VAlignOptions vAlign=MiddleAligned);
  void drawGlyph(char glyph);
  void initializeGlyph(char ch);
  double getWidth(char ch);
  double getHeight(char ch);
  vector2d getSize(char ch);
  double getWidth(const char* str);
  double getHeight(const char* str);
  double getAscent(char ch);
  double getDescent(char ch);
  double getAscent(const char* str);
  double getDescent(const char* str);
  
private:
  static const char* getPrimitiveType(GLenum type);
  static void tessBeginCB(GLenum which);
  static void tessEndCB();
  static void tessVertexCB(const GLvoid *data);
  static void tessErrorCB(GLenum errorCode);
  static const double FontRenderSize;
};

#endif //GL_TEXT_H
