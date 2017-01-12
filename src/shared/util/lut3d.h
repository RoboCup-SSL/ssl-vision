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
  \file    lut3d.h
  \brief   C++ Interface: LUT3D
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef LUT3D_H
#define LUT3D_H
#include "colors.h"
#include "conversions.h"
#include <assert.h>
#include <vector>
#include <string>
#include <qmutex.h>
#include "VarTypes.h"
#define LUTFILL_MAXDEPTH 10000
#define LUTFILL_PUSH(XL, XR, Y, DY) \
    if( sp < stack+LUTFILL_MAXDEPTH && Y+(DY) >= 0 && Y+(DY) <= getMaxZ() ) \
    { sp->xl = XL; sp->xr = XR; sp->y = Y; sp->dy = DY; ++sp; }
#define LUTFILL_POP(XL, XR, Y, DY) \
    { --sp; XL = sp->xl; XR = sp->xr; Y = sp->y+(DY = sp->dy); }
    

/// We allow 32 distinct channels (bitwise)
typedef uint8_t lut_mask_t;

using namespace std;
using namespace VarTypes;

enum LUTChannelMode {
  LUTChannelMode_Numeric, //each YUV color is mapped to exactly one channel. Channels are numerated
  LUTChannelMode_Bitwise  //each YUV color is mapped to a bitmask, each bit represents a channel
};

struct LINESEGMENT { int xl, xr, y, dy; } ;

/*!
  \class LUTChannel
  \brief  A text and color-label for a channel used in the LUT3D class
  \author Stefan Zickler
*/

class LUTChannel {
  public:
  LUTChannel() {
    label="Unnamed";
    draw_color.set(0,0,0);
  }
  LUTChannel(string l, rgb c) {
    label=l;
    draw_color=c;
  }
  string label;
  rgb draw_color;
};

/*!
  \class LUT3D
  \brief  A general 3D LUT class, allowing fast bit-wise lookup
  \author Stefan Zickler
*/
class LUT3D : public QObject {
  Q_OBJECT
  protected:
  public:
    unsigned int X_BITS; //number of index bits for x-dimension
    unsigned int Y_BITS; //number of index bits for y-dimension
    unsigned int Z_BITS; //number of index bits for z-dimension

    //derived values:
    unsigned int X_SHIFT; //how many bits to truncate the x-dimension by when storing in LUT
    unsigned int Y_SHIFT; //how many bits to truncate the y-dimension by when storing in LUT
    unsigned int Z_SHIFT; //how many bits to truncate the z-dimension by when storing in LUT

    unsigned int Y_SHIFT_PLUS_8;
    unsigned int Z_SHIFT_PLUS_16;
    
    unsigned int Z_AND_Y_BITS;
    unsigned int TOTAL_BITS; //total number of index bits
    unsigned int LUT_SIZE; //total size of LUT in bytes

    lut_mask_t * LUT;
    VarBlob * v_blob;
    VarList * v_settings;
    vector<LUTChannel> channels;
    vector<LUT3D *> derived_LUTs;
    QMutex mutex;
  protected slots:
    void slotVBlobChange() {
      updateDerivedLUTs();
    }
  public:
    //set filename to "" if this LUT should not be stored.
    LUT3D(unsigned int x_bits=7, unsigned int y_bits=7, unsigned int z_bits=7, string filename="3dlut.xml") {
      assert(x_bits <= 8 && y_bits <= 8 && z_bits <= 8);
      X_BITS = x_bits; // bits for each field, should be less or equal to 8
      Y_BITS = y_bits; // bits for each field, should be less or equal to 8
      Z_BITS = z_bits; // bits for each field, should be less or equal to 8

      //derived values:
      X_SHIFT=8-X_BITS;
      Y_SHIFT=8-Y_BITS;
      Z_SHIFT=8-Z_BITS;

      //Y_SHIFT_PLUS_8=Y_SHIFT + 8;
      //Z_SHIFT_PLUS_16=Z_SHIFT + 16;

      Z_AND_Y_BITS = Y_BITS+Z_BITS; // bits for each field
      TOTAL_BITS = X_BITS + Y_BITS + Z_BITS; // bits for each field
      //LUT_SIZE = (0x1 << (TOTAL_BITS+1)) - 0x01;
      LUT_SIZE = (0x01 << (TOTAL_BITS+1));// + 1;
      channels.resize(sizeof(lut_mask_t));
      LUT=new lut_mask_t[LUT_SIZE];

      if (filename=="") {
        v_settings=0;
        v_blob=0;
      } else {
        v_settings=new VarExternal(filename,"LUT 3D");
        v_settings->addChild(v_blob=new VarBlob((uint8_t *)LUT,(int)LUT_SIZE*sizeof(lut_mask_t),"LUT Data"));
        connect(v_blob,SIGNAL(XMLwasRead(VarType *)),this,SLOT(slotVBlobChange()));
      }

      reset();
    };

    void lock() {
      mutex.lock();
    }
    void unlock() {
      mutex.unlock();
    }
    VarList * getSettings() {
      return v_settings;
    }

    LUTChannel getChannel(unsigned int idx) const {
      if (idx >= channels.size()) {
        fprintf(stderr,"invalid channel selected in getChannel(...)\n");
      }
      return channels[idx];

    }

    void clearDerivedLUTs(bool unallocate_derived_memory=true) {
      lock();
        int n = derived_LUTs.size();
        if (unallocate_derived_memory) {
          for (int i = 0; i < n; i ++) {
            delete derived_LUTs[i];
          }
        }
        derived_LUTs.clear();
      unlock();
    }

    int getDerivedLUTcount() {
      return derived_LUTs.size();
    }

    LUT3D * getDerivedLUT(int idx) {
      LUT3D * res=0;
      lock();
        res = derived_LUTs[idx];
      unlock();
      return res;
    }

    void addDerivedLUT(LUT3D * lut) {
      lock();
      if (lut!=0) {
        derived_LUTs.push_back(lut);
      }
      unlock();
    }

    LUT3D * getDerivedLUT(ColorSpace space) {
     LUT3D * result=0;
     lock();
      int n = derived_LUTs.size();
      for (int i = 0; i < n; i ++) {
        if (derived_LUTs[i]->getColorSpace()==space) {
          result= derived_LUTs[i];
          break;
        }
      }
      unlock();
      return result;
    }

    void updateDerivedLUTs() {
      lock();
      int n = derived_LUTs.size();
      for (int i = 0; i < n; i ++) {
        derived_LUTs[i]->copyChannels(*this);
        derived_LUTs[i]->deriveFromLUT(this);
      }
      unlock(); 
    }

    virtual void deriveFromLUT(LUT3D * lut) {
      for (int x=0;x<=255;x++) {
        for (int y=0;y<=255;y++) {
          for (int z=0;z<=255;z++) {
            set((unsigned char)x,(unsigned char)y,(unsigned char)z,lut->get((unsigned char)x,(unsigned char)y,(unsigned char)z));
          }
        }
      }
    }

    int getChannelID(const string & label) const {
      for (int i = 0; i < getChannelCount(); i++) {
        if (channels[i].label.compare(label)==0) return i;
      }
      return -1;
    }

    void setChannel(unsigned int idx, LUTChannel c) {
      if (idx < channels.size()) {
        channels[idx]=c;
      } else {
        fprintf(stderr,"invalid channel selected in getChannel(...)\n");
      }
    }

    int getChannelCount() const {
      return channels.size();
    }

    int getSizeX() const {
      return ((0x01 << (X_BITS)));
    }

    int getSizeY() const {
      return ((0x01 << (Y_BITS)));
    }

    int getSizeZ() const {
      return ((0x01 << (Z_BITS)));
    }

    int getMaxX() const {
      return ((0x01 << (X_BITS)) - 0x01);
    }

    int getMaxY() const {
      return ((0x01 << (Y_BITS)) - 0x01);
    }

    int getMaxZ() const {
      return ((0x01 << (Z_BITS)) - 0x01);
    }


    virtual ~LUT3D() {
      channels.clear();
      clearDerivedLUTs(true);
      delete[] LUT;
      if (v_blob!=0) delete v_blob;
      if (v_settings!=0) delete v_settings;
    };

    virtual ColorSpace getColorSpace() const {
      return CSPACE_UNDEFINED;
    }

    void reset() {
      lock();
      memset(LUT,0x00,LUT_SIZE*sizeof(lut_mask_t));
      unlock();
    };

    lut_mask_t * getTable() const {
      return LUT;
    }

    inline lut_mask_t * getPointer(unsigned char x, unsigned char y,unsigned char z) const {
      return LUT + (((x >> X_SHIFT) << Z_AND_Y_BITS) | ((y >> Y_SHIFT) << Z_BITS) | (z >> Z_SHIFT));
    }

    inline lut_mask_t * getPointerPreshrunk(unsigned char x, unsigned char y,unsigned char z) const {
      return LUT + (((x) << Z_AND_Y_BITS) | ((y) << Z_BITS) | (z));
    }

    inline unsigned char norm2lutX(unsigned char x) const {
      return(x >> X_SHIFT);
    }
    inline unsigned char norm2lutY(unsigned char y) const {
      return(y >> Y_SHIFT);
    }
    inline unsigned char norm2lutZ(unsigned char z) const {
      return(z >> Z_SHIFT);
    }
    inline unsigned char lut2normX(unsigned char x) const {
      return(x << X_SHIFT);
    }
    inline unsigned char lut2normY(unsigned char y) const {
      return(y << Y_SHIFT);
    }
    inline unsigned char lut2normZ(unsigned char z) const {
      return(z << Z_SHIFT);
    }

    inline void set(unsigned char x, unsigned char y,unsigned char z, lut_mask_t mask) {
      LUT[((x >> X_SHIFT) << Z_AND_Y_BITS) | ((y >> Y_SHIFT) << Z_BITS) | (z >> Z_SHIFT)]=mask;
    }

    inline void set_preshrunk(unsigned char x, unsigned char y,unsigned char z, lut_mask_t mask) {
      LUT[((x) << Z_AND_Y_BITS) | ((y) << Z_BITS) | (z)]=mask;
    }

    /*inline lut_mask_t getXYZuint24(const uint32_t & val) {
      return LUT[(((val >> X_SHIFT) & 0xFF) << Z_AND_Y_BITS) | (((val >> Y_SHIFT_PLUS_8) & 0xFF) << Z_BITS) | ((val >> Z_SHIFT_PLUS_16) & 0xFF)];
    }*/

    inline lut_mask_t get(const unsigned char x, const unsigned char y,const unsigned char z) {
      return LUT[((x >> X_SHIFT) << Z_AND_Y_BITS) | ((y >> Y_SHIFT) << Z_BITS) | (z >> Z_SHIFT)];
    }

    inline lut_mask_t get_preshrunk(const unsigned char x, const unsigned char y,const unsigned char z) {
      return LUT[((x) << Z_AND_Y_BITS) | ((y) << Z_BITS) | (z)];
    }

    void copyChannels(const LUT3D & other) {
      lock();
      int n=other.getChannelCount();
      channels.clear();
      for (int i=0;i<n;i++) {
        channels.push_back(other.getChannel(i));
      }
      unlock();
    }

    void loadRoboCupChannels(LUTChannelMode mode) {
      lock();
      channels.clear();
      if (mode==LUTChannelMode_Numeric) channels.push_back(LUTChannel("<Clear>",RGB::Black));
      channels.push_back(LUTChannel("Field Green",RGB::DarkGreen));
      channels.push_back(LUTChannel("Orange",RGB::Orange));
      channels.push_back(LUTChannel("Yellow",RGB::Yellow));
      channels.push_back(LUTChannel("Blue",RGB::Blue));
      channels.push_back(LUTChannel("Pink",RGB::Pink));
      channels.push_back(LUTChannel("Cyan",RGB::Cyan));
      channels.push_back(LUTChannel("Green",RGB::Green));
      channels.push_back(LUTChannel("White",RGB::White));
      channels.push_back(LUTChannel("Black",RGB::Black));
      unlock();
    }

    void loadBlackWhite(LUTChannelMode mode) {
      lock();
      channels.clear();
      if (mode==LUTChannelMode_Numeric) channels.push_back(LUTChannel("<Clear>",RGB::Black));
      channels.push_back(LUTChannel("White",RGB::White));
      channels.push_back(LUTChannel("Black",RGB::Black));
      unlock();
    }


    void maskFillYZ(unsigned char slice_x, unsigned char origin_y, unsigned char origin_z,
                            lut_mask_t new_color, LUTChannelMode mode, bool remove
                              = false, bool check_exact = true, bool write_exclusive = false)
    {

      /// This function fills an area in the y/z plane of an LUT for a given x-slice
      /// if check_exact is true then we only extend to pixels matching exactly the origin's mask
      ///                if it's false then we extend to pixels matching any one of the origin's mask's channels
      /// if write_exclusive is true then we set the target pixel to be exactly the new_color
      ///              if it's false then we OR the target pixel with the new_color
      /// the special case is where new_color is 0 in which we go into erase mode.
      ///              in this case if write_exclusive is false, we will subtract the origins color's from the current pixel
      /// Note that the values of check_exact and write_exclusive only matter if mode is
      /// LUTChannelMode_Bitwise. They are ignored if mode is LUTChannelMode_Numeric.
      if (mode==LUTChannelMode_Numeric) {
        check_exact=true;
        write_exclusive=true;
      }

      int x=origin_y;
      int y=origin_z;
      int left, x1, x2, dy;
      int x_max_index=getMaxY();
      int y_max_index=getMaxZ();

      lut_mask_t old_color=get_preshrunk(slice_x,x,y);
      //if (remove) old_color=old_color & new_color;

      LINESEGMENT stack[LUTFILL_MAXDEPTH], *sp = stack;

      if (remove==false) {
        //see if the origin is already colored with newcolor
        if ( check_exact ? (old_color == new_color) : ((old_color & new_color) != 0x00) ) return;
      }
      else {
        //see if the origin is already fully removed of newcolor
        if (mode==LUTChannelMode_Bitwise) {
          if ( (old_color & new_color) == 0x00 ) return;
        } else {
          if ( old_color==0x00 ) return;
        }
      }

      if( (x < 0) || (x > x_max_index) || (y < 0) || (y > y_max_index) ) return;

      LUTFILL_PUSH(x, x, y, 1);        /* needed in some cases */
      LUTFILL_PUSH(x, x, y+1, -1);    /* seed segment (popped 1st) */

      while( sp > stack ) {
        LUTFILL_POP(x1, x2, y, dy);

        //for( x = x1; x >= 0 && (check_exact ? (get_preshrunk(slice_x,x,y) == old_color) : ((get_preshrunk(slice_x,x,y) & old_color) != 0x00)); --x )
        for( x = x1; x >= 0 && (check_exact ? (get_preshrunk(slice_x,x,y) == old_color) : (((get_preshrunk(slice_x,x,y) & new_color) == 0x00) == (remove ? false : true)) ); --x )
          (remove ?
            ((mode==LUTChannelMode_Bitwise) ? 
              (set_preshrunk(slice_x,x,y,get_preshrunk(slice_x,x,y) & (~new_color))) :
              (set_preshrunk(slice_x,x,y,0x00)))
              :
            (write_exclusive ?
              set_preshrunk(slice_x,x,y,new_color) :
              set_preshrunk(slice_x,x,y,get_preshrunk(slice_x,x,y) | (new_color))));

        if( x >= x1 ) goto SKIP;

        left = x+1;
        if( left < x1 )
          //BUG FIXED by S.Zickler
          //OLD CODE WAS: LUTFILL_PUSH(y, left, x1-1, -dy);    /* leak on left? */
          LUTFILL_PUSH(left, x1-1, y, -dy); /* leak on left? */

        x = x1+1;

        do {
          //for( ; x<=getMaxY() && (check_exact ? (get_preshrunk(slice_x,x,y) == old_color) : ((get_preshrunk(slice_x,x,y) & old_color) != 0x00)); ++x )
          for( ; x<=getMaxY() && (check_exact ? (get_preshrunk(slice_x,x,y) == old_color) : (((get_preshrunk(slice_x,x,y) & new_color) == 0x00) == (remove ? false : true))); ++x )
            (remove ?
              ((mode==LUTChannelMode_Bitwise) ? 
              (set_preshrunk(slice_x,x,y,get_preshrunk(slice_x,x,y) & (~new_color))) :
              (set_preshrunk(slice_x,x,y,0x00)))
              :
              (write_exclusive ?
                set_preshrunk(slice_x,x,y,new_color) :
                set_preshrunk(slice_x,x,y,get_preshrunk(slice_x,x,y) | (new_color))));


          LUTFILL_PUSH(left, x-1, y, dy);

          if( x > x2+1 ) LUTFILL_PUSH(x2+1, x-1, y, -dy);    /* leak on right? */

        SKIP:
          for( ++x; x <= x2 && ((check_exact ? (get_preshrunk(slice_x,x,y) == old_color) : (((get_preshrunk(slice_x,x,y) & new_color) == 0x00) == (remove ? false : true)))==false); ++x ) {
            ;
          }

          left = x;
        } while( x<=x2 );
      }
    }
};


/*!
  \class RGBLUT
  \brief  A 3D RGB LUT
  \author Stefan Zickler
*/
class RGBLUT : public LUT3D {
  public:
  RGBLUT(unsigned int r_bits=5, unsigned int g_bits=5, unsigned int b_bits=5, string filename="rgblut.xml") : LUT3D(r_bits, g_bits, b_bits,filename) {};
  virtual void deriveFromLUT(LUT3D * lut) {
    if (lut->getColorSpace()!=CSPACE_YUV) {
      fprintf(stderr,"Warning: deriveFromLUT input on RGBLUT does not seem to be in YUV color-space\n");
    } else {
      // OLD :
      /*int y,u,v;
      for (int r=0;r<=255;r++) {
        for (int g=0;g<=255;g++) {
          for (int b=0;b<=255;b++) {
            Conversions::rgb2yuv(r,g,b,y,u,v);
            set((unsigned char)r,(unsigned char)g,(unsigned char)b,lut->get((unsigned char)y,(unsigned char)u,(unsigned char)v));
          }
        }
      }*/
      
      //new, faster:
      int y,u,v;
      int rn=this->getSizeX();
      int gn=this->getSizeY();
      int bn=this->getSizeZ();
      for (int r=0;r!=rn;r++) {
        for (int g=0;g!=gn;g++) {
          for (int b=0;b!=bn;b++) {
            Conversions::rgb2yuv((int)lut2normX((unsigned char)r),(int)lut2normY((unsigned char)g),(int)lut2normZ((unsigned char)b),y,u,v);
            set_preshrunk(r,g,b,lut->get((unsigned char)y,(unsigned char)u,(unsigned char)v));
          }
        }
      }
      
    }
  }

  virtual ColorSpace getColorSpace() const {
    return CSPACE_RGB;
  }

};

/*!
  \class YUVLUT
  \brief  A 3D YUV LUT
  \author Stefan Zickler
*/
class YUVLUT : public LUT3D {
  public:
  YUVLUT(unsigned int y_bits=4, unsigned int u_bits=5, unsigned int v_bits=5, string filename="yuvlut.xml") : LUT3D(y_bits, u_bits, v_bits,filename) {};

  virtual void deriveFromLUT(LUT3D * lut) {
    if (lut->getColorSpace()!=CSPACE_RGB) {
      fprintf(stderr,"Warning: deriveFromLUT input on YUVLUT does not seem to be in RGB color-space\n");
    } else {
      /*int r,g,b;
      for (int y=0;y<=255;y++) {
        for (int u=0;u<=255;u++) {
          for (int v=0;v<=255;v++) {
            Conversions::yuv2rgb(y,u,v,r,g,b);
            set((unsigned char)y,(unsigned char)u,(unsigned char)v,lut->get((unsigned char)r,(unsigned char)g,(unsigned char)b));
          }
        }
      }*/
      int r,g,b;
      int yn=this->getSizeX();
      int un=this->getSizeY();
      int vn=this->getSizeZ();
      for (int y=0;y!=yn;y++) {
        for (int u=0;u!=un;u++) {
          for (int v=0;v!=vn;v++) {
            Conversions::yuv2rgb((int)lut2normX((unsigned char)y),(int)lut2normY((unsigned char)u),(int)lut2normZ((unsigned char)v),r,g,b);
            set_preshrunk((unsigned char)y,(unsigned char)u,(unsigned char)v,lut->get((unsigned char)r,(unsigned char)g,(unsigned char)b));
          }
        }
      }
    }
  }

  /// This will clear the LUT and create a new LUT-dataset modeling a NN-lookup based solely on color labels
  virtual void computeLUTfromLabels(int max_dist=0) {
    this->lock();
    long max_dist_sq=max_dist * max_dist;
    int cn=channels.size();
    long best_dist=0;
    int best_idx=0;
    yuv l;
    for (int y=0;y<=getMaxX();y++) {
      for (int u=0;u<=getMaxY();u++) {
        for (int v=0;v<=getMaxZ();v++) {
          best_idx=0;
          best_dist=0;
          for (int i = 1; i <cn; i++) {
              l = Conversions::rgb2yuv(channels[i].draw_color);
              long dist=(sq( lut2normX(y)-l.y)+sq(lut2normY(u)-l.u)+sq(lut2normZ(v)-l.v));
              if (i==1 || (dist < best_dist)) {
                best_dist=dist;
                best_idx=i;
              }
              if (max_dist_sq!=0 && best_dist > max_dist_sq) {
                best_idx=0;
              }
          }
            //Conversions::yuv2rgb(y,u,v,r,g,b);
          set_preshrunk((unsigned char)y,(unsigned char)u,(unsigned char)v,best_idx);
        }
      }
    }
    this->unlock();
  }
  virtual ColorSpace getColorSpace() const {
    return CSPACE_YUV;
  }

};


#endif
