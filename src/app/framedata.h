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
  \file    framedata.h
  \brief   C++ Interface: FrameData
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================


#ifndef FRAMEDATA_H
#define FRAMEDATA_H
#include "ringbuffer.h"
#include "rawimage.h"
#include <map>
using namespace std;

/*!
  \class   FrameDataMap
  \brief   A general storage map, for plugins to store and read their data
  \author  Stefan Zickler, (C) 2008

  This class acts as a storage map of string and data-pointer pairs.
  This allows any plugin to make its results publicly available to the
  entire image stack pipeline for the current frame.
*/
class FrameDataMap : protected map<string,void *>
{
public:
  void * get(const string & label) const {
    map<string,void *>::const_iterator iter = map<string,void *>::find(label);
    if (iter==map<string,void *>::end()) return 0;
    return iter->second;
  }
  void * insert(const string & label, void * item) {
    pair< map<string,void *>::iterator, bool > pair = map<string,void *>::insert ( make_pair(label,item) );
    if (pair.first==map<string,void *>::end()) return 0;
    return pair.first->second;
  }
};

/*!
  \class   FrameData
  \brief   A class to store any data related to the current frame.
  \author  Stefan Zickler, (C) 2008

  This class acts as the main storage class for any data related to the current frame.
  This includes the frame itself, stored in the \p video RawImage
  Any additional data can be stored in the FrameDataMap \p map
*/
class FrameData
{
public:
  long long number;
  int cam_id;
  double time;
  RawImage video;//the video image from the camera (input)

  FrameDataMap map; //all other data

  FrameData();

  ~FrameData();
};

/*!
  \class   FrameBuffer
  \brief   A RingBuffer consisting of items of type FrameData
  \author  Stefan Zickler, (C) 2008
*/
typedef RingBuffer<FrameData> FrameBuffer;

#endif
