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
  \file    ringbuffer.h
  \brief   C++ Interface: RingBuffer
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_
#include <qmutex.h>

/*!
  \class RingBuffer
  \brief A template-based ring-buffer class
  \author  Stefan Zickler, (C) 2008

  A ringbuffer is a thread-safe data-structure which allows
  the coordination of a reader and a writer to temporary, sequential data.

  The most typical use is for processing live-streaming data.
  E.g. a network thread might receive data and writes it to the ringbuffer
    our software then reads the ringbuffer and displays the data on the screen

  Another common application is the processing of video-data.

  The ringbuffer ensures that
    1) the reader and the writer will NEVER access the same bin at the same time
    2) the reader will not leap ahead over the writer
*/

template <class ITEM>
class RingBuffer {
  protected:
    QMutex mutex;
    QMutex readlock;
  public:
    ITEM * items;
  private:
    int current_read;
    int current_write;
    int previous_write;
    int previous_read;
  public:
    int size;
    /*!
      \brief Constructor of the Ringbuffer
      \param _size determines how many elements are stored in it.

      Note that \p _size needs to be at least 2!
    */
    RingBuffer ( int _size ) {
      items=new ITEM[_size];

      current_read=0;
      previous_read=-1;
      previous_write=0;
      current_write=1;
      size=_size;

    }
    virtual ~RingBuffer() {
      delete[] items;
    }
  private:
    int next ( int cur_idx, int not_avail, bool preventLap ) {
      //if preventLap is true then we won't jump over
      //not_avail.
      int idx=cur_idx+1;
      if ( idx >= size ) idx=0;
      if ( idx < 0 ) idx=0;
      if ( idx == not_avail ) {
        if ( preventLap ) {
          idx=cur_idx;
        } else {
          idx++;
        }
      }
      if ( idx >= size || idx==-1 ) idx=0;
      return idx;
    }
  public:

    /*!
      \brief returns the item (or a copy thereof) at index \p idx
    */
    ITEM getItem ( int idx ) {
      if ( idx >= size ) idx=size-1;
      if ( idx < 0 ) idx=0;
      return items[idx];
    }

    /*!
      \brief returns a pointer to the item at index \p idx
    */
    ITEM * getPointer ( int idx ) {
      if ( idx >= size ) idx=size-1;
      if ( idx < 0 ) idx=0;
      return & ( items[idx] );
    }

    /*!
      \brief gets the index to the next write-bin

      If \p allow_lapping is true then this function can overtake the
      read-pointer. This means that we are overwriting old frames which
      have not been read yet.
      if \p allow_lapping is false then we will not overatake a read-pointer.
      Instead, we will stay at our current location and thus return the same
      index as on the previous call. this means that our caller can overwrite
      the most recently written frame with the actually most recent frame.

      nextWrite is thread-safe and atomic.
    */
    int nextWrite ( bool allow_lapping ) {
      mutex.lock();
      previous_write=current_write;
      current_write=next ( current_write,current_read, allow_lapping );
      int res=current_write;
      mutex.unlock();
      return res;
    }

    /*!
      \brief gets the index to the next read-bin

      If \p skip_frames is true then this will return the bin that was most
      recently written. In other words, we always jump directly ahead to the most
      recent data available. This means that we can possibly skip bins.

      If \p skip_frames is false then we only move to the next read-bin in order
      (we will not jump ahead). Select this, if you want to ensure that all data is read.

      Either way, nextRead always ensures that we will never overlap the write-pointer.
      Thus, it is possible that nextRead will return the same index as on the previous call
      if we are unable to move on because the write-pointer is in front of us.

      nextRead is thread-safe and atomic.
    */
    int nextRead ( bool skip_frames ) {
      mutex.lock();
      previous_read=current_read;
      if ( skip_frames ) {
        int prev_frame;
        if ( previous_write==0 ) {
          prev_frame=size-1;
        } else {
          prev_frame=previous_write-1;
          if ( prev_frame < 0 ) prev_frame=0;
        }
        current_read=next ( prev_frame,current_write,true );
      } else {
        current_read=next ( current_read,current_write,true );
      }
      int res=current_read;
      mutex.unlock();
      return res;
    }

    /*!
      \brief returns the index of the current write-bin
    */
    int curWrite() {
      int res;
      mutex.lock();
      res=current_write;
      mutex.unlock();
      return res;
    }

    /*!
      \brief returns the index of the current read-bin
    */
    int curRead() {
      int res;
      mutex.lock();
      res=current_read;
      mutex.unlock();
      return res;
    }

    /*!
      \brief locks the readlock mutex
      This function is only required for scenarios where you expect to have
      *multiple readers*, all accessing the same ringbuffer.
    */
    void lockRead() {
      readlock.lock();
    }

    /*!
      \brief unlocks the readlock mutex
      This function is only required for scenarios where you expect to have
      *multiple readers*, all accessing the same ringbuffer.
    */
    void unlockRead() {
      readlock.unlock();
    }
};

#endif /*RINGBUFFER_H_*/
