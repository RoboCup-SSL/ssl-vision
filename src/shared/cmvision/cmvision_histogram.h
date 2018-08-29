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
  \file    cmvision_histogram.h
  \brief   C++ Interface: cmvision_histogram
  \author Stefan Zickler, 2009 (inspired by original CMVision Histogram code by James Bruce)
*/
//========================================================================
#ifndef CMVISION_HISTOGRAM_H
#define CMVISION_HISTOGRAM_H
#include "image.h"

namespace CMVision {

class Histogram{
protected:
    int * channels;
    int max_channels;
public:
    Histogram(int _max_channels);
    ~Histogram();

    //will sample a rectangular bounding box of a color-labeled image and add it to the histogram
    //the return value is the area of the box.
    int addBox(const Image<raw8> * image, int x1, int y1, int x2, int y2);
    int getChannel(int channel);
    void setChannel(int channel, int value);
    void clear();
};

}

#endif
