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
  \file    global_random.cpp
  \brief   C++ Implementation: GlobalRandom
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#include "global_random.h"

GlobalRandom* GlobalRandom::pinstance = 0;// initialize pointer

Random* GlobalRandom::getInstance ()
{
  if (pinstance == 0)  // is it the first call?
  {
    pinstance = new GlobalRandom; // create sole instance
  }
  return pinstance->tool; // address of sole instance
}

GlobalRandom::GlobalRandom()
{
  tool = new Random();
  tool->randomize();
}
