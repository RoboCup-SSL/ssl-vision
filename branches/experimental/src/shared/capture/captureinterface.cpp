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
  \file    captureinterface.h
  \brief   C++ Implementation: CaptureInterface
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "captureinterface.h"

CaptureInterface::CaptureInterface(VarList * _settings)
{
  if (_settings!=0) {
    settings=_settings;
  } else {
    settings=new VarList("capture settings");
  }
}


CaptureInterface::~CaptureInterface()
{
  delete settings;
}


bool CaptureInterface::resetBus() {
  return true;
}

void CaptureInterface::readAllParameterValues() {

}

bool CaptureInterface::copyAndConvertFrame(const RawImage & src, RawImage & target) {
  target.ensure_allocation(target.getColorFormat(),src.getWidth(),src.getHeight());
  target.setTime(src.getTime());
  memcpy(target.getData(),src.getData(),src.getNumBytes());
  return true;
}
