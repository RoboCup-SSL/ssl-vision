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
  \file    framecounter.h
  \brief   Generic framerate / frame counting class
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef FRAMECOUNTER_H_
#define FRAMECOUNTER_H_

#include <QTime>
#include <QMutex>


/*!
  \brief A class for frame counting and frame-rate measurements
*/
class FrameCounter {
protected:	
	int interval;
	
	double fps;
	long long total;
	long long framecounter;
	QTime timer;
	QMutex mutex;
public:
	FrameCounter(int _interval=1000) {
		reset(_interval);
	}
	~FrameCounter() {}
	void reset(int _interval=1000) {
		mutex.lock();
		interval=_interval;
		framecounter=0;
		fps=0.0;
		total=0;
		timer.restart();
		mutex.unlock();
	}
	void count() {
		mutex.lock();
		framecounter++;
		total++;
		mutex.unlock();
	}
	void count(long long val) {
		mutex.lock();
		framecounter+=val;
		total+=val;
		mutex.unlock();
	}
	long long getTotal() {
		long long res;
		mutex.lock();
		res=total;
		mutex.unlock();
		return res;
	}
	double getFPS(bool & changed) {
		mutex.lock();
		if (timer.elapsed() > 1000) {
    	double secs=((double)timer.restart())/1000.0;
      fps=((double)framecounter)/secs;
    	framecounter=0;
    	changed=true;
    } else {
    	changed=false;
		}
		double res=fps;
		mutex.unlock();
    return res;
	}    
};
#endif /*FRAMECOUNTER_H_*/
