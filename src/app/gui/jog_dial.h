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
  \file    jog_dial.h
  \brief   C++ Interface: JogDial
  \author  Roman Shtylman, 2009
*/
//========================================================================
#ifndef jog_dial_H
#define jog_dial_H

#include <QWidget>
#include <QMutex>


class JogDial : public QWidget
{
  Q_OBJECT;
  
  public:
    JogDial(QWidget* parent = 0);
    
    /** returns the normalized offset */
    float offset() { 
      float result=0.0;
      mutex.lock();
      result=_offset*2.0f/_viewSpan;
      mutex.unlock();
      return result;
    }
    
  Q_SIGNALS:
    /** emitted when the offset changes, normalized */
    void valueChanged(float offset);
    
  protected:
    QMutex mutex;
    void paintEvent(QPaintEvent* pe);
    void mousePressEvent(QMouseEvent* me);
    void mouseMoveEvent(QMouseEvent* me);
    void mouseReleaseEvent(QMouseEvent* me);
    
  private:
    /** angle offset from start (not normalized)*/
    float _offset;
    
    /** degrees between dial marks */
    float _spacing;
    
    /** range of angles to see */
    float _viewSpan;
    
    /** start x coordinate for mouse drag */
    int _x;
};


#endif
