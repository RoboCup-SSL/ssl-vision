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
  \file    TimeLine.h
  \brief   C++ Interface: TimeLine
  \author  Stefan Zickler, (C) 2008
*/
#ifndef TIMELINE_H_
#define TIMELINE_H_
#include <QList>
#include <QListIterator>
#include <QMutex>
#include "ClosedRange.h"
#include "enhanced/TimeVar.h"
#include "VarTypes.h"
#include "TimePointer.h"


//NOTE: a timeline should normally be of template type TimeVarTemplate<vDataType>
class TimeLine : public VarType {
    Q_OBJECT
  protected:
    TimePointer * cur;
    QList<TimeVar *> list;
  public slots:
    void slotPointerChange() {
      qDebug ( "TIME CHANGED\n" );
      changed();
    }
  public:
    TimeLine ( string name = "", TimePointer * current = 0 ) : VarType ( name ) {
      setTimePointer ( current );
      //list.append(new type);
    };

    TimeVar * at ( int id ) const {
      if ( id < 0 || id >= list.size() ) return 0;
      return list.at ( id );
    }

    int size() const {
      return list.size();
    }

    ValueClosedRange getValueClosedRange() const {

      ValueClosedRange result;
      QListIterator<TimeVar *> i ( list );
      bool first = true;
      while ( i.hasNext() ) {
        TimeVar  * val = i.next();
        if ( val != 0 ) {
          //FIXME: TODO: use the provided hasValue hasMax getMaxValue...functions
          if ( first || ( val->getData()->getValue() < result.getMin() ) ) result.setMin ( val->getData()->getValue() );
          if ( first || ( val->getData()->getValue() > result.getMax() ) ) result.setMax ( val->getData()->getValue() );
          first = false;
        }
      }
      return result;
    }

    void append ( TimeVar * value ) {
      list.append ( value );

      changed();
    }
    TimeClosedRange getTimeClosedRange() const {

      TimeClosedRange result;
      QListIterator<TimeVar *> i ( list );
      while ( i.hasNext() ) {
        TimeVar * val = ( TimeVar  * ) i.next();
        if ( val != 0 ) {
          if ( ( *val ) < result.getMin() ) result.setMin ( ( TimeIndex ) ( *val ) );
          if ( ( *val ) > result.getMax() ) result.setMax ( ( TimeIndex ) ( *val ) );
        }
      }
      return result;
    }


    void setTimePointer ( TimePointer * tp ) {
      if ( cur != 0 ) {
        disconnect ( tp, SIGNAL ( hasChanged(VarType *) ), this, SLOT ( slotPointerChange() ) );
      }
      qDebug ( "setup pointer\n" );
      cur = tp;
      connect ( tp, SIGNAL ( hasChanged(VarType *) ), this, SLOT ( slotPointerChange() ) );
      changed();
    }

    TimePointer * getTimePointer() const {
      return cur;
    }

    TimeVar * find ( TimeIndex t ) const {
      int idx = findIdx ( t );
      if ( idx == ( -1 ) ) return 0;
      return ( list.at ( idx ) );
    }

    int findIdx ( TimeIndex idx )  const {
      //perform a binary search for the index
      int maxidx = list.size() - 1;
      int minidx = 0;
      int range;
      int split;

      if ( maxidx < 0 ) return -1;

      while ( true ) {
        range = maxidx - minidx;
        split = ( range / 2 ) + minidx;
        if ( range <= 1 ) return split;

        if ( * ( ( TimeIndex * ) ( list.at ( split ) ) ) == idx ) {
          return split;
        } else if ( * ( ( TimeIndex * ) ( list.at ( split ) ) ) < idx ) {
          minidx = split;
        } else {
          maxidx = split;
        }
      }
    }


    TimeVar * findCurrent() const {
      if ( cur == 0 ) {
        return find ( 0.0 );
      } else {
        return find ( cur->get() );
      }
    }


    virtual string getString() const { return ""; }
    virtual void resetToDefault() { list.clear(); };
    virtual void printdebug() const { printf ( "TimeLine named %s containing %d element(s)\n", getName().c_str(), list.size() ); }

    virtual VarTypeId getType() const { return DT_TIMELINE; };
    virtual ~TimeLine() {};


    static bool timePointerLessThan ( const TimeVar * s1, const TimeVar * s2 ) {
      if ( s1 != 0 && s2 != 0 ) {
        return ( *s1 ) < ( *s2 );
      } else {
        return false;
      }
    }


    virtual void sort() {
      qSort ( list.begin(), list.end(), timePointerLessThan );
      changed();
    }

    virtual vector<VarType *> getChildren() const {
      vector<VarType *> v;
      //findCurrent();
      VarType * tmp = ( VarType * ) findCurrent();
      if ( tmp != 0 ) {
        v.push_back ( tmp );
      }
      return v;
    }

#ifndef VDATA_NO_XML
  protected:
    virtual void readChildren ( XMLNode & us ) {
      qDebug ( "updating children\n" );
      vector<VarType *> v;
      QListIterator<TimeVar *> i ( list );
      while ( i.hasNext() ) {
        v.push_back ( ( VarType * ) i.next() );
      }

      vector<VarType *> result;

      result = readChildrenHelper ( us, v, false, true );

      list.clear();
      for ( unsigned int i = 0;i < result.size();i++ ) {
        qDebug ( "found one\n" );
        if ( result[i]->getType() == DT_TIMEVAR ) {
          list.append ( ( TimeVar * ) result[i] );
        }
      }
      sort();
    }

    virtual void updateChildren ( XMLNode & us ) const {
      //wipe out all children of type var...
      //us.deleteNodeContent(1);
      deleteAllVarChildren ( us );
      QListIterator<TimeVar *> i ( list );
      while ( i.hasNext() ) {
        ( i.next() )->writeXML ( us, true );
      }
    }

#endif

    //getChildren()//will return the children matching closest to the current timepointer.
};

#endif /*TIMELINE_H_*/

