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
  \file    range.h
  \brief   Template-based class representing a numeric range
  \author  James R. Bruce <jbruce@cs.cmu.edu>, (C) 2007
*/
//========================================================================
#ifndef _INCLUDED_RANGE_H_
#define _INCLUDED_RANGE_H_

namespace Range {

#define RANGE_TEM template <class num,bool open_min,bool open_max>
#define RANGE_FUN Range<num,open_min,open_max>

/*!
  \class  Range
  \brief  Template-based class representing a numeric range
  \author James R. Bruce <jbruce@cs.cmu.edu>, (C) 2007
*/
RANGE_TEM
class Range{
public:
  num min;
  num max;
public:
  void set(num _min,num _max)
    {min=_min; max=_max;}
  void set(num _min_max)
    {min=_min_max; max=_min_max;}
  void expand(num e)
    {min-=e; max+=e;}
  bool inside(num v) const;
};

RANGE_TEM
bool RANGE_FUN::inside(num v) const
{
  return((open_min? v>min : v>=min) &&
         (open_max? v<max : v<=max));
}

#undef RANGE_TEM
#undef RANGE_FUN
};
#endif
