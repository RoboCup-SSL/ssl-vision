//
// C++ Interface: framelimiter
//
// Description: This class can be used to limit the framerate of your game.
//
//
// Author: Stefan Zickler <szickler@cs.cmu.edu>, 2007
//
//
#ifndef FRAMELIMITER_H
#define FRAMELIMITER_H
#include "timer.h"
#include <stdio.h>

class FrameLimiter{
private:
  double TLastDraw;
  double TLastOverShoot;
  double desired_fps;
  bool was_init;
  Timer timer;
public:

  FrameLimiter() {
    was_init=false;
  };

  //init needs to be called *once* with the desired framerate
  void init(double fps) {
    desired_fps=fps;
    TLastDraw=timer.midtime();
    TLastOverShoot=timer.midtime();
    timer.start();
    was_init=true;
  };

  ~FrameLimiter() {};

  //waitForNextFrame() should be called each time right before the buffer switch
  void waitForNextFrame() {
    if (was_init==false) {
      printf("Need to call init() first before waitForNextFrame()\n");
    }
    // Wait if we are ahead of the desired framerate
    double waiting=1.0 / desired_fps;
    double curtime=timer.midtime();
    double diff=waiting-(curtime-TLastDraw);
    if (diff > 0) {
    //Compensate for any previous sluggish Frames:
      if (TLastOverShoot > 0 && TLastOverShoot < diff) diff-=TLastOverShoot;
      if (diff > waiting) diff=waiting;
      usleep((unsigned long)(diff*1.0E6));
    }
    TLastDraw=timer.midtime();
    TLastOverShoot=(TLastDraw-curtime)-diff;
  };
};

#endif
