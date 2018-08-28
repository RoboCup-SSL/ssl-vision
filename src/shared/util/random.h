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
  \file    random.h
  \brief   C++ Interface: Random
  \author  Makoto Matsumoto and Takuji Nishimura, 1997-2002
*/
//========================================================================

/*
   C++ class definition for use with random.cc This class implements a
   Mersenne Twister RNG, which is not only fast but has very good
   independence properties.  See the following for details:
     http://www.math.keio.ac.jp/~matumoto/emt.html

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above
        copyright notice, this list of conditions and the following
        disclaimer in the documentation and/or other materials
        provided with the distribution.

     3. The names of its contributors may not be used to endorse or
        promote products derived from this software without specific
        prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
   COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
   OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stdint.h>

/*!
  \class Random
  \brief A class implementing the Mersenne Twister RNG
*/
class Random{
  static const int N = 624;
  static const int M = 397;

  // state
  uint32_t state[N];
  uint32_t *next;
  int left;

  int gleft;
  double grand;

  void next_state();
public:
  Random(){left=-1;}

  void randomize();
  void seed(uint32_t s);
  void seed(const uint32_t *init_key,int key_length);

  // uniform random numbers
  unsigned uint32();
  int int32()
    {return((int)(uint32()));}
  int int31()
    {return((int)(uint32()>>1));}

  unsigned uint32(unsigned max) // unsigned integer in [0,max)
    {return(uint32() % max);}
  unsigned uint32(int min, int max) // unsigned integer in [min,max)
    {return(uint32() % (max-min) + min);}

  double sreal32() // real number in [-1,1)
    {return((double)uint32() * (2.0/4294967296.0) - 1.0);}
  double real32() // real number in [0,1)
    {return((double)uint32() * (1.0/4294967296.0));}
  double real53()
    {unsigned a=uint32()>>5,b=uint32()>>6;
     return(a*67108864.0+b)*(1.0/9007199254740992.0);}

  // non-uniform distributions
  double gaussian32();
};

#endif /*__RANDOM_H__*/
