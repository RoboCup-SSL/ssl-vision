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
  \file    nvector.h
  \brief   Vector class for general constant size vectors
  \author  James R. Bruce <jbruce@cs.cmu.edu>, (C) 2004
*/
//========================================================================

#ifndef __NVECTOR_H__
#define __NVECTOR_H__

#include <math.h>
#include "util.h"

namespace Vec {

template <class num,const int dim>
class NVector{
public:
  num v[dim];
public:
  num &operator[](int i)
    {return(v[i]);}
  const num &operator[](int i) const
    {return(v[i]);}
public:
  int size() {return(dim);}

  template <class num2>
  void set(num2 val);

  void copy(const NVector<num,dim> &a);
  void zero() {set(0);}
  void unit(int dir);
  num sqlength();
  num length() {return(sqrt(sqlength()));}

  void add(const NVector<num,dim> &a);
  void sub(const NVector<num,dim> &a);
  void mul(const NVector<num,dim> &a);
  void div(const NVector<num,dim> &a);

  void add(const NVector<num,dim> &a,const NVector<num,dim> &b);
  void sub(const NVector<num,dim> &a,const NVector<num,dim> &b);
  void mul(const NVector<num,dim> &a,const NVector<num,dim> &b);
  void div(const NVector<num,dim> &a,const NVector<num,dim> &b);

  template <class num2>
  void mul(const NVector<num,dim> &a,num2 s);
  template <class num2>
  void div(const NVector<num,dim> &a,num2 s);

  template <class num2>
  void mul(num2 s);
  template <class num2>
  void div(num2 s);

  void neg(const NVector<num,dim> &a) {mul(a,-1);}
  void neg() {mul(-1);}
  void norm(const NVector<num,dim> &a) {mul(a,1/a.length());}
  void norm() {mul(1/length());}

  void min(const NVector<num,dim> &a,const NVector<num,dim> &b);
  void max(const NVector<num,dim> &a,const NVector<num,dim> &b);
  void abs_bound(const NVector<num,dim> &a,const NVector<num,dim> &b);
  void bound(const NVector &a,const NVector &b,const NVector &c);
};


template <class num,const int dim> template <class num2>
void NVector<num,dim>::set(num2 val)
{
  switch(dim){
    case 4: v[3] = val;
    case 3: v[2] = val;
    case 2: v[1] = val;
    case 1: v[0] = val;
      break;
    default:
      for(int i=0; i<dim; i++){
        v[i] = val;
      }
  }
}

template <class num,const int dim>
void NVector<num,dim>::copy(const NVector<num,dim> &a)
{
  switch(dim){
    case 4: v[3] = a.v[3];
    case 3: v[2] = a.v[2];
    case 2: v[1] = a.v[1];
    case 1: v[0] = a.v[0];
      break;
    default:
      for(int i=0; i<dim; i++){
        v[i] = a.v[i];
      }
  }
}

template <class num,const int dim>
void NVector<num,dim>::unit(int dir)
{
  switch(dim){
    case 4: v[3] = (dir == 3);
    case 3: v[2] = (dir == 2);
    case 2: v[1] = (dir == 1);
    case 1: v[0] = (dir == 0);
      break;
    default:
      for(int i=0; i<dim; i++){
        v[i] = (dir == i);
      }
  }
}

template <class num,const int dim>
num NVector<num,dim>::sqlength()
{
  num sum = 0;
  switch(dim){
    case 4: sum += v[3] * v[3];
    case 3: sum += v[2] * v[2];
    case 2: sum += v[1] * v[1];
    case 1: sum += v[0] * v[0];
      break;
    default:
      for(int i=0; i<dim; i++){
        sum += v[i] * v[i];
      }
  }
  return(sum);
}

#define NVECTOR_EQUAL_BINARY_OPERATOR(fname,opr) \
  template <class num,const int dim> \
  void NVector<num,dim>::fname(const NVector<num,dim> &a) \
  {                               \
    switch(dim){                  \
      case 4: v[3] opr a.v[3];    \
      case 3: v[2] opr a.v[2];    \
      case 2: v[1] opr a.v[1];    \
      case 1: v[0] opr a.v[0];    \
        break;                    \
      default:                    \
        for(int i=0; i<dim; i++){ \
          v[i] opr a.v[i];        \
        }                         \
    }                             \
  }

NVECTOR_EQUAL_BINARY_OPERATOR(add,+=)
NVECTOR_EQUAL_BINARY_OPERATOR(sub,-=)
NVECTOR_EQUAL_BINARY_OPERATOR(mul,*=)
NVECTOR_EQUAL_BINARY_OPERATOR(div,/=)


#define NVECTOR_BINARY_OPERATOR(fname,opr) \
  template <class num,const int dim>       \
  void NVector<num,dim>::fname(            \
             const NVector<num,dim> &a,    \
             const NVector<num,dim> &b)    \
  {                                        \
    switch(dim){                           \
      case 4: v[3] = a.v[3] opr b.v[3];    \
      case 3: v[2] = a.v[2] opr b.v[2];    \
      case 2: v[1] = a.v[1] opr b.v[1];    \
      case 1: v[0] = a.v[0] opr b.v[0];    \
        break;                             \
      default:                             \
        for(int i=0; i<dim; i++){          \
          v[i] = a.v[i] opr b.v[i];        \
        }                                  \
    }                                      \
  }

NVECTOR_BINARY_OPERATOR(add,+)
NVECTOR_BINARY_OPERATOR(sub,-)
NVECTOR_BINARY_OPERATOR(mul,*)
NVECTOR_BINARY_OPERATOR(div,/)


#define NVECTOR_SCALAR_BINARY_OPERATOR(fname,opr) \
  template <class num,const int dim> template <class num2> \
  void NVector<num,dim>::fname(         \
             const NVector<num,dim> &a, \
             num2 s)                    \
  {                                     \
    switch(dim){                        \
      case 4: v[3] = a.v[3] opr s;      \
      case 3: v[2] = a.v[2] opr s;      \
      case 2: v[1] = a.v[1] opr s;      \
      case 1: v[0] = a.v[0] opr s;      \
        break;                          \
      default:                          \
        for(int i=0; i<dim; i++){       \
          v[i] = a.v[i] opr s;          \
        }                               \
    }                                   \
  }

NVECTOR_SCALAR_BINARY_OPERATOR(mul,*)
NVECTOR_SCALAR_BINARY_OPERATOR(div,/)


#define NVECTOR_SCALAR_EQUAL_BINARY_OPERATOR(fname,opr) \
  template <class num,const int dim> template <class num2> \
  void NVector<num,dim>::fname(num2 s) \
  {                                    \
    switch(dim){                       \
      case 4: v[3] opr s;              \
      case 3: v[2] opr s;              \
      case 2: v[1] opr s;              \
      case 1: v[0] opr s;              \
        break;                         \
      default:                         \
        for(int i=0; i<dim; i++){      \
          v[i] opr s;                  \
        }                              \
    }                                  \
  }

NVECTOR_SCALAR_EQUAL_BINARY_OPERATOR(mul,*=)
NVECTOR_SCALAR_EQUAL_BINARY_OPERATOR(div,/=)


#define NVECTOR_BINARY_SIMD_FUNCTION(fname) \
  template <class num,const int dim>        \
  void NVector<num,dim>::fname(             \
             const NVector<num,dim> &a,     \
             const NVector<num,dim> &b)     \
  {                                         \
    switch(dim){                            \
      case 4: v[3] = fname(a.v[3], b.v[3]); \
      case 3: v[2] = fname(a.v[2], b.v[2]); \
      case 2: v[1] = fname(a.v[1], b.v[1]); \
      case 1: v[0] = fname(a.v[0], b.v[0]); \
        break;                              \
      default:                              \
        for(int i=0; i<dim; i++){           \
          v[i] = fname(a.v[i], b.v[i]);     \
        }                                   \
    }                                       \
  }

NVECTOR_BINARY_SIMD_FUNCTION(min)
NVECTOR_BINARY_SIMD_FUNCTION(max)
NVECTOR_BINARY_SIMD_FUNCTION(abs_bound)

#define NVECTOR_TERNARY_SIMD_FUNCTION(fname)  \
  template <class num,const int dim>          \
  void NVector<num,dim>::fname(               \
             const NVector<num,dim> &a,       \
             const NVector<num,dim> &b,       \
             const NVector<num,dim> &c)       \
  {                                           \
    switch(dim){                              \
      case 4: v[3] = ::fname(a.v[3], b.v[3], c.v[3]); \
      case 3: v[2] = ::fname(a.v[2], b.v[2], c.v[2]); \
      case 2: v[1] = ::fname(a.v[1], b.v[1], c.v[1]); \
      case 1: v[0] = ::fname(a.v[0], b.v[0], c.v[0]); \
        break;                                        \
      default:                                        \
        for(int i=0; i<dim; i++){                     \
          v[i] = ::fname(a.v[i], b.v[i], c.v[i]);     \
        }                                             \
    }                                                 \
  }

NVECTOR_TERNARY_SIMD_FUNCTION(bound)

// non-member functions

template <class num,const int dim>
num sqdist(const NVector<num,dim> &a,const NVector<num,dim> &b)
{
  num sum = 0;
  switch(dim){
    case 4: sum += sq(a.v[3] - b.v[3]);
    case 3: sum += sq(a.v[2] - b.v[2]);
    case 2: sum += sq(a.v[1] - b.v[1]);
    case 1: sum += sq(a.v[0] - b.v[0]);
      break;
    default:
      for(int i=0; i<dim; i++){
        sum += sq(a.v[i] - b.v[i]);
      }
  }
  return(sum);
}

template <class num,const int dim>
num dist(const NVector<num,dim> &a,const NVector<num,dim> &b)
{
  return(sqrt(sqdist(a,b)));
}

template <class num,const int dim>
num dot(const NVector<num,dim> &a,const NVector<num,dim> &b)
{
  num sum = 0;
  switch(dim){
    case 4: sum += a.v[3] * b.v[3];
    case 3: sum += a.v[2] * b.v[2];
    case 2: sum += a.v[1] * b.v[1];
    case 1: sum += a.v[0] * b.v[0];
      break;
    default:
      for(int i=0; i<dim; i++){
        sum += a.v[i] * b.v[i];
      }
  }
  return(sum);
}

}; // namespace Vec

#endif
