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
  \file    cmvision_histogram.cpp
  \brief   C++ Implementation: cmvision_histogram
  \author Stefan Zickler, 2009 (inspired by original CMVision Histogram code by James Bruce)
*/
//========================================================================
#include "cmvision_histogram.h"

namespace CMVision {

Histogram::Histogram(int _max_channels)
{
  if (_max_channels < 1) _max_channels=1;
  max_channels=_max_channels;
  channels=new int[max_channels];
}

void Histogram::clear() {
  for (int i=0;i<max_channels;i++) {
    channels[i]=0;
  }
}

int Histogram::addBox(const Image<raw8> * image, int x1, int y1, int x2, int y2) {
  raw8 * data = image->getPixelData();
  int image_width = image->getWidth();
  int image_height = image->getHeight();

  x1 = bound(x1,0,image_width-1);
  y1 = bound(y1,0,image_height-1);
  x2 = bound(x2,0,image_width-1);
  y2 = bound(y2,0,image_height-1);

  for(int y=y1; y<=y2; y++){
    for(int x=x1; x<=x2; x++){
      channels[data[y*image_width+x].v]++;
    }
  }

  return((x2 - x1 + 1) * (y2 - y1 + 1));
}

int Histogram::getChannel(int channel) {
  return channels[channel];
}

void Histogram::setChannel(int channel, int value) {
  channels[channel]=value;
}

Histogram::~Histogram()
{
  delete[] channels;
}

};

