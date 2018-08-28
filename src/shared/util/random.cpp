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
  \file    random.cpp
  \brief   C++ Implementation: Random
  \author  Makoto Matsumoto and Takuji Nishimura, 1997-2002
*/
//========================================================================

/*
   MT19937, with initialization improved 2002/2/10.
   Coded by Takuji Nishimura and Makoto Matsumoto.
   This is a faster version by taking Shawn Cokus's optimization,
   Matthe Bellew's simplification, Isaku Wada's real version.
   Converted to C++ class by James Bruce (jbruce <at> cs.cmu.edu)

   Before using, initialize the state by using seed(seedval)
   or array version seed(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

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

   Any feedback about this generator is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto <at> math.keio.ac.jp

   Send implementation questions/comments to jbruce <at> cs.cmu.edu
   since the original authors aren't responsible for this C++ conversion.
*/

#include <cstdlib>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "random.h"

#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UMASK 0x80000000UL /* most significant w-r bits */
#define LMASK 0x7fffffffUL /* least significant r bits */
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

using std::size_t;

void Random::randomize()
{
  timeval tv;
  uint32_t a[2];
  int fd;

  fd = open("/dev/urandom",O_RDONLY);
  if(fd >= 0){
    const size_t num_bytes = sizeof(uint32_t)*N;
    if (read(fd,state,num_bytes) != num_bytes) {
      fprintf(stderr, "WARNING: Unable to read sufficient seed bytes from "
              "/dev/urandom");
    }
    close(fd);

    left = 1;
  }else{
    #if WIN32
    //FIXME.
    seed(43);
    #else
    gettimeofday(&tv,NULL);
    a[0] = tv.tv_sec;
    a[1] = tv.tv_usec;
    seed(a,2);
    #endif
  }
}

void Random::seed(uint32_t s)
{
  int j;

  state[0]= s;
  for(j=1; j<N; j++){
    state[j] = (1812433253UL * (state[j-1] ^ (state[j-1] >> 30)) + j);
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array state[].                     */
    /* 2002/01/09 modified by Makoto Matsumoto             */
  }

  left = 1;
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
void Random::seed(const uint32_t *init_key,int key_length)
{
  int i,j,k;

  seed(19650218UL);
  i=1; j=0;
  k = (N>key_length ? N : key_length);

  for(; k; k--){
    state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1664525UL))
      + init_key[j] + j; /* non linear */
    i++; j++;
    if(i>=N){ state[0] = state[N-1]; i=1; }
    if(j>=key_length) j=0;
  }
  for(k=N-1; k; k--){
    state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1566083941UL))
      - i; /* non linear */
    i++;
    if(i>=N){ state[0] = state[N-1]; i=1; }
  }

  state[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
  left = 1;
}

void Random::next_state()
{
  uint32_t *p=state;
  int j;

  /* if seed() or randomize() has not been called, */
  /* a default initial seed is used         */
  if(left < 0) seed(5489UL);

  left = N;
  next = state;

  for(j=N-M+1; --j; p++){
    *p = p[M] ^ TWIST(p[0], p[1]);
  }

  for(j=M; --j; p++){
    *p = p[M-N] ^ TWIST(p[0], p[1]);
  }

  *p = p[M-N] ^ TWIST(p[0], state[0]);
}

unsigned Random::uint32()
{
  unsigned y;

  if(--left <= 0) next_state();
  y = *next++;

  /* Tempering */
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);

  return(y);
}

double Random::gaussian32()
{
  double x,y,l,f;

  if(gleft){
    gleft = 0;
    return(grand);
  }else{
    do{
      x = 2*real32()-1;
      y = 2*real32()-1;
      l = x*x + y*y;
    }while(l>=1.0 || l==0.0);

    f = sqrt(-2*log(l)/l);
    gleft = 1;
    grand = y*f;
    return(x*f);
  }
}
