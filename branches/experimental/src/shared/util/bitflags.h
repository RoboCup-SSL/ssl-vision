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
  \file    bitflags.h
  \brief   A template class providing common bitwise flag operations
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef BITFLAGS_H_
#define BITFLAGS_H_
#include <inttypes.h>
#include <sys/types.h>

template <class num = uint32_t>
class BitFlags
{
protected:
  num data;
public:

  inline bool isRaised(num const & val) const
  {
    return ((data & val) == val);
  };
  inline bool isDropped(num const & val) const
  {
    return ((data & val) == 0x00);
  };
  inline bool areAnyRaised(num const & val) const
  {
    return ((data & val) != 0x00);
  };
  inline bool areAnyDropped(num const & val) const
  {
    return ((data & val) != val);
  }
  inline void set(num const & val)
  {
    data=val;
  };
  inline num get() const
    {
      return data;
    }
  inline bool isClear()
  {
    return (data==0x00);
  }
  inline void clear()
  {
    data=0x00;
  };
  inline void drop(num const & val)
  {
    data&=(~val);
  };
  inline void raise(num const & val)
  {
    data|=val;
  };

  BitFlags()
  {
    clear();
  }

  BitFlags(const num & p)
  {
    data=p;
  }

  inline void operator=(const BitFlags<num> & p)
  {
    data=p.get();
  }
  inline bool operator==(const BitFlags<num> & p) const
  {
    return data==p.get();
  }
  inline bool operator!=(const BitFlags<num> & p) const
  {
    return !operator==(p);
  }
  inline BitFlags<num> operator&(const BitFlags<num> & p) const
  {
    return data&p.get();
  }
  inline BitFlags<num> operator|(const BitFlags<num> & p) const
  {
    return data|p.get();
  }
  inline BitFlags<num> operator^(const BitFlags<num> & p) const
  {
    return data^p.get();
  }
  inline BitFlags<num> &operator&=(const BitFlags<num> & p) const
  {
    data&=p;
    return (*this);
  }
  inline BitFlags<num> &operator|=(const BitFlags<num> & p) const
  {
    data|=p;
    return (*this);
  }
  inline BitFlags<num> &operator^=(const BitFlags<num> & p) const
  {
    data^=p;
    return (*this);
  }
  inline BitFlags<num> operator~() const
  {
    BitFlags<num> res;
    res.data=~get();
    return res;
  }
  inline void operator=(const num & p)
  {
    data=p;
  }
  inline bool operator==(const num & p) const
  {
    return data==p;
  }
  inline bool operator!=(const num & p) const
  {
    return !operator==(p);
  }
  inline num operator&(const num & p) const
  {
    return data&p;
  }
  inline num operator|(const num & p) const
  {
    return data|p.get();
  }
  inline num operator^(const num & p) const
  {
    return data^p;
  }
  inline num &operator&=(const num & p) const
  {
    data&=p;
    return (*this);
  }
  inline num &operator|=(const num & p) const
  {
    data|=p;
    return (*this);
  }
  inline num &operator^=(const num & p) const
  {
    data^=p;
    return (*this);
  }

};

typedef uint8_t bitflag8;
typedef uint16_t bitflag16;
typedef uint32_t bitflag32;
typedef uint64_t bitflag64;
typedef BitFlags<bitflag8> BitFlags8;
typedef BitFlags<bitflag16> BitFlags16;
typedef BitFlags<bitflag32> BitFlags32;
typedef BitFlags<bitflag64> BitFlags64;
#endif /*BITFLAGS_H*/
