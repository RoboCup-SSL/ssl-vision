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

  \file    timer.h
  \brief   Interval timers, cycle counters, and time utilities
  \author  James R. Bruce, (C) 1999-2002
*/
//========================================================================

#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/time.h>
#include <time.h>


/*!
  \class Timer
  \brief a basic timer class
  \author  James R. Bruce, (C) 1999-2002
*/
class Timer{
  timeval tv1,tv2;
public:
  void start()  {gettimeofday(&tv1,NULL);}
  void stop()   {gettimeofday(&tv2,NULL);}
  void end()    {stop();}
  double time() {return((tv2.tv_sec - tv1.tv_sec) +
                        (tv2.tv_usec - tv1.tv_usec) * 1.0E-6);}
  double timeMSec() {return(time() * 1.0E3);}
  double timeUSec() {return(time() * 1.0E6);}

  double interval(){
    double t;
    gettimeofday(&tv2,NULL);
    t = time();
    tv1 = tv2;
    return(t);
  }
  double midtime() {
    timeval tmp;
    gettimeofday(&tmp,NULL);
    return((tmp.tv_sec - tv1.tv_sec) +
                        (tmp.tv_usec - tv1.tv_usec) * 1.0E-6);
  }
};


/*!
  \class AccumulativeTimer
  \brief a basic timer class
  \author  James R. Bruce, (C) 1999-2002
*/
class AccumulativeTimer{
  timeval tv1,tv2;
  double total;
public:
  AccumulativeTimer() {
    clear();
  }
  void clear() {
    total=0.0;
  }
  void start()  {gettimeofday(&tv1,NULL);}
  void stop()   {
    gettimeofday(&tv2,NULL);
    total+=time();
  }
  double getTotal() {
    return total;
  }
  void end()    {stop();}
  double time() {return((tv2.tv_sec - tv1.tv_sec) +
                        (tv2.tv_usec - tv1.tv_usec) * 1.0E-6);}
  double timeMSec() {return(time() * 1.0E3);}
  double timeUSec() {return(time() * 1.0E6);}

  double interval(){
    double t;
    gettimeofday(&tv2,NULL);
    t = time();
    tv1 = tv2;
    return(t);
  }
  double midtime() {
    timeval tmp;
    gettimeofday(&tmp,NULL);
    return((tmp.tv_sec - tv1.tv_sec) +
                        (tmp.tv_usec - tv1.tv_usec) * 1.0E-6);
  }
};

typedef uint32_t cycle_t;
typedef uint64_t cycle64_t;

// Access clock cycle counter on i386 compatibles
#ifdef __i386__

#define get_cycle(cnt) \
  __asm__ __volatile__("rdtsc" : "=a" (cnt) : : "edx")

#define get_cycle64(cnt) \
     __asm__ __volatile__("rdtsc" : "=A" (cnt))

#endif

#ifdef __x86_64__

#define get_cycle(cnt) \
    __asm__ __volatile__("rdtsc" : "=A" (cnt))

#define get_cycle64(cnt) \
    __asm__ __volatile__("rdtsc" : "=A" (cnt))

#endif


// The following is untested
#ifdef Apertos
#define get_cycle(cp0r9) \
  __asm__ __volatile__("mfc0 %0, $9" :: "=r" (cp0r9));
#endif

inline unsigned GetTimeUSec()
{
#ifdef Apertos
  struct SystemTime time;
  GetSystemTime(&time);
  return(time.seconds*1000000 + time.useconds);
#else
  timeval tv;
  gettimeofday(&tv,NULL);
  return(tv.tv_sec*1000000 + tv.tv_usec);
#endif
}

inline double GetTimeSec()
{
#ifdef Apertos
  struct SystemTime time;
  GetSystemTime(&time);
  return((double)time.seconds + time.useconds*(1.0E-6));
#else
  timeval tv;
  gettimeofday(&tv,NULL);
  return((double)tv.tv_sec + tv.tv_usec*(1.0E-6));
#endif
}

inline void GetDate(struct tm &date)
{
  time_t t = time(NULL);
  localtime_r(&t,&date);
}

// returns CPU clock rate in MHz
double GetCPUClockRateMHz();

// returns CPU clock period in sec
double GetCPUClockPeriod();


class CycleTimer{
  cycle_t c1,c2;
  static double cpu_period;
public:
  CycleTimer() {cpu_period=GetCPUClockPeriod();}

  void start()  {get_cycle(c1);}
  void stop()   {get_cycle(c2);}
  void end()    {stop();}

  unsigned cycles() {return(c2 - c1);}
  double time()     {return((double)(c2-c1) * cpu_period * 1.0E0);}
  double timeMSec() {return((double)(c2-c1) * cpu_period * 1.0E3);}
  double timeUSec() {return((double)(c2-c1) * cpu_period * 1.0E6);}
};

template<int num>
class StageCycleTimer{
  cycle_t c[num+1];
  unsigned n;
public:
  void start()
    {n=0; get_cycle(c[n]);}
  void stage()
    {if(n<num){n++; get_cycle(c[n]);}}
  void print(FILE *out=stdout);
  int cycles(int stage)
    {return((int)(c[stage+1] - c[stage]));}
};

template<int num>
void StageCycleTimer<num>::print(FILE *out)
{
  if(num){
    cycle_t tot = c[n] - c[0];
    for(unsigned i=0; i<n; i++){
      cycle_t dc = c[i+1] - c[i];
      fprintf(out,"  %d: %5.2f%% %d\n",i,100.0*dc/tot,dc);
    }
  }
}

inline void Sleep(double sec)
{
  usleep((int)(sec * 1E6));
}

class FunctionTimer{
  const char *fname;
  Timer timer;
public:
  FunctionTimer(const char *_fname)
    {fname=_fname; timer.start();}
  ~FunctionTimer()
    {timer.stop(); printf("%s: %0.3f\n",fname,timer.timeMSec());}
};

class FunctionCycleTimer{
  const char *fname;
  CycleTimer timer;
public:
  FunctionCycleTimer(const char *_fname)
    {fname=_fname; timer.start();}
  ~FunctionCycleTimer()
    {timer.stop(); printf("%s: %0.3f\n",fname,timer.timeMSec());}
};

#endif
