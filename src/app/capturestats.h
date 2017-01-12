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
  \file    capturestats.h
  \brief   A class for storing capture statistics.
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef CAPTURESTATS_H_
#define CAPTURESTATS_H_

/*!
  \class CaptureStats
  \brief   A class for storing capture statistics.
  \author  Stefan Zickler, (C) 2008
*/
class CaptureStats {
  public:
  double fps_capture;
  long long total;
  CaptureStats() {
    fps_capture=0.0;
    total=0;
  }
};

#endif /*CAPTURESTATS_H_*/
