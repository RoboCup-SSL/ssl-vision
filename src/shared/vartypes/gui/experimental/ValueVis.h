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
  \file    ValueVis.h
  \brief   C++ Interface: ValueVis
  \author  Stefan Zickler, (C) 2008
*/
#ifndef VALUEVIS_H_
#define VALUEVIS_H_
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include "VarTypes.h"

class ValueVis : public QFrame
{
protected:
  VarType * dt;
  QVBoxLayout * layout;
  QLabel * label;
  QLabel * title;
        QList<ValueVis *> * children;
public:
  ValueVis(VarType * _dt);
  virtual ~ValueVis();
        void setupUi();
  void update(bool recurse=true);
};

#endif /*VALUEVIS_H_*/
