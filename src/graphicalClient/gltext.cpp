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
\brief   C++ Implementation: GLText
\author  Joydeep Biswas (C) 2011
*/
//========================================================================

#include "gltext.h"

const bool GLText::debugTesselation = false;
const double GLText::FontRenderSize = 1000.0;

GLText::GLText(QFont _font)
{
  glyphs.clear();
  font = _font;
  font.setPixelSize(FontRenderSize);
  characterSpacing = 0.1;
}

GLText::~GLText()
{
  
}

void GLText::drawGlyph(char glyph)
{
  if(glyphs.size()<glyph+1)
    glyphs.resize(glyph+1);
  if(!glyphs[glyph].compiled)
    initializeGlyph(glyph);
  glCallList(glyphs[glyph].displayListID);
}

double GLText::getWidth(char ch)
{
  if(glyphs.size()<ch+1)
    glyphs.resize(ch+1);
  if(!glyphs[ch].compiled)
    initializeGlyph(ch);
  return glyphs[ch].width;
}

double GLText::getHeight(char ch)
{
  if(glyphs.size()<ch+1)
    glyphs.resize(ch+1);
  if(!glyphs[ch].compiled)
    initializeGlyph(ch);
  return glyphs[ch].height;
}

vector2d GLText::getSize(char ch)
{
  return vector2d(getWidth(ch),getHeight(ch));
}

double GLText::getWidth(const char* str)
{
  double width = 0.0;
  while(*str>0){
    width += getWidth(*str);
    str++;
    if((*str)>0){
      width += characterSpacing;
    }
  }
  return width;
}

double GLText::getHeight(const char* str)
{
  double height = 0.0;
  while(*str>0){
    height = max(height,getHeight(*str));
    str++;
  }
  return height;
}

double GLText::getAscent(char ch)
{
  if(glyphs.size()<ch+1)
    glyphs.resize(ch+1);
  if(!glyphs[ch].compiled)
    initializeGlyph(ch);
  return glyphs[ch].ascent;
}

double GLText::getDescent(char ch)
{
  if(glyphs.size()<ch+1)
    glyphs.resize(ch+1);
  if(!glyphs[ch].compiled)
    initializeGlyph(ch);
  return glyphs[ch].descent;
}

double GLText::getAscent(const char* str)
{
  double ascent = 0.0;
  while(*str>0){
    ascent = max(ascent,getAscent(*str));
    str++;
  }
  return ascent;
}

double GLText::getDescent(const char* str)
{
  double descent = 0.0;
  while(*str>0){
    descent = max(descent,getDescent(*str));
    str++;
  }
  return descent;
}

void GLText::drawString(vector2d loc, double angle, double size, const char* str, HAlignOptions hAlign, VAlignOptions vAlign)
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslated(loc.x,loc.y,0.0);
  glScaled(size, size,1.0);
  glRotated(angle,0,0,1);
  
  switch(hAlign){
    case LeftAligned:{
      //Normal rendering will achieve this!
      break;
    }
    case RightAligned:{
      glTranslated(-getWidth(str),0,0);
      break;
    }
    case CenterAligned:{
      glTranslated(-0.5*getWidth(str),0,0);
      break;
    }
  }
  switch(vAlign){
    case BottomAligned:{
      glTranslated(0.0,getDescent(str),0.0);
      break;
    }
    case TopAligned:{
      glTranslated(0.0,-getAscent(str),0.0);
      break;
    }
    case MedianAligned:{
      //Normal rendering will achieve this!
      break;
    }
    case MiddleAligned:{
      glTranslated(0.0,-0.5*getHeight(str),0.0);
      break;
    }
  }
  vector2d textDir;
  textDir.heading(0);
  double d = 0.0;
  while(*str>0){
    drawGlyph(*str);
    d = characterSpacing + 0.5*getWidth(*str);
    str++;
    if((*str)>0){
      d += 0.5*getWidth(*str);
      glTranslated(d*textDir.x,d*textDir.y,0.0);
    }
  }
  glPopMatrix();
}

void GLText::initializeGlyph(char ch)
{
  Glyph glyph;
  QPainterPath path;
  if(debugTesselation) printf("Adding glyph %c\n",ch);
  path.addText(0,0,font,QString((QChar)ch));
  QList<QPolygonF> polygons = path.toSubpathPolygons();
  if(debugTesselation){
    printf("%d Sub-Polygons\n",polygons.size());
  printf("Poly has %d vertices:\n",polygons.size());
  }
  int numVertices = 0;
  double minX=DBL_MAX, minY=DBL_MAX, maxX=-DBL_MAX, maxY=-DBL_MAX;
  for(int i=0; i<polygons.size(); i++){
    if(debugTesselation) printf("Sub-Polygon %d:\n",i);
    numVertices += polygons[i].size();
    
    for(int k=0; k<polygons[i].size(); k++){
      if(debugTesselation) printf("%8.3f,%8.3f\n",polygons[i][k].x(),polygons[i][k].y());
      minX = min(minX, polygons[i][k].x());
      maxX = max(maxX, polygons[i][k].x());
      minY = min(minY, polygons[i][k].y());
      maxY = max(maxY, polygons[i][k].y());
    }
  }
  glyph.ascent = fabs(minY)/FontRenderSize;
  glyph.descent = fabs(maxY)/FontRenderSize;
  
  glyph.height = (maxY - minY)/FontRenderSize;
  glyph.width = (maxX - minX)/FontRenderSize;
  
  if(debugTesselation) printf("numVertices: %d\n",numVertices);
  GLdouble vertices[numVertices][3];
  int j=0;
  for(int i=0; i<polygons.size(); i++){
    for(int k=0; k<polygons[i].size(); k++){
      vertices[j][0] = polygons[i][k].x()/FontRenderSize;
      vertices[j][1] = -polygons[i][k].y()/FontRenderSize;
      vertices[j][2] = 9;
      j++;
    }
  }
  
  GLUtesselator* tess = gluNewTess();
  gluTessCallback(tess, GLU_TESS_BEGIN, (_GLUfuncptr) tessBeginCB);
  gluTessCallback(tess, GLU_TESS_END, (_GLUfuncptr) tessEndCB);
  gluTessCallback(tess, GLU_TESS_ERROR, (_GLUfuncptr) tessErrorCB);
  gluTessCallback(tess, GLU_TESS_VERTEX, (_GLUfuncptr) tessVertexCB);
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
  glyph.displayListID = glGenLists(1);
  if(glyph.displayListID==GL_INVALID_VALUE){
    printf("Unable to create display list!\n");
    exit(1);
  }
  glNewList(glyph.displayListID, GL_COMPILE);
  gluTessBeginPolygon(tess, 0);
  j=0;
  for(int i=0; i<polygons.size(); i++){
    gluTessBeginContour(tess);
    for(int k=0; k<polygons[i].size(); k++){
      gluTessVertex(tess, vertices[j], vertices[j]);
      j++;
    }
    gluTessEndContour(tess);
  }
  gluTessEndPolygon(tess);
  gluDeleteTess(tess);
  glEndList();
  glPopMatrix();
  glyph.compiled = true;
  glyphs[ch] = glyph;
}

const char* GLText::getPrimitiveType(GLenum type)
{
  switch(type)
  {
    case 0x0000:
      return "GL_POINTS";
      break;
    case 0x0001:
      return "GL_LINES";
      break;
    case 0x0002:
      return "GL_LINE_LOOP";
      break;
    case 0x0003:
      return "GL_LINE_STRIP";
      break;
    case 0x0004:
      return "GL_TRIANGLES";
      break;
    case 0x0005:
      return "GL_TRIANGLE_STRIP";
      break;
    case 0x0006:
      return "GL_TRIANGLE_FAN";
      break;
    case 0x0007:
      return "GL_QUADS";
      break;
    case 0x0008:
      return "GL_QUAD_STRIP";
      break;
    case 0x0009:
      return "GL_POLYGON";
      break;
  }
  return "UNKNOWN";
}

void GLText::tessBeginCB(GLenum which)
{
  glBegin(which);
  if(debugTesselation) printf("glBegin(%s);\n",getPrimitiveType(which));
}

void GLText::tessEndCB()
{
  glEnd();
  if(debugTesselation) printf("glEnd();\n");
}

void GLText::tessVertexCB(const GLvoid *data)
{
  const GLdouble *ptr = (const GLdouble*)data;
  if(debugTesselation) printf("glVertex3d(%f,%f,%f);\n",*ptr,*(ptr+1),*(ptr+2));
  glVertex3dv(ptr);
}

void GLText::tessErrorCB(GLenum errorCode)
{
  const GLubyte *errorStr;
  errorStr = gluErrorString(errorCode);
  printf("[ERROR]: %s\n", errorStr);
}
