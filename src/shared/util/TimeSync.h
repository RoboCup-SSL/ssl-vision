#ifndef SSL_VISION_TIMESYNC_H
#define SSL_VISION_TIMESYNC_H

#include <deque>
#include <sys/time.h>
#include <iostream>
#include <cmath>

class TimeSync {

public:
  TimeSync();

  void update(uint64_t timestamp);
  uint64_t sync(uint64_t timestamp) const;

private:
  int64_t currentOffset;
  std::deque<int64_t> offsetBuffer;
  std::deque<int64_t> diffBuffer;

  static int64_t calcOffset(uint64_t tRef, uint64_t tOther);
  static int64_t average(const std::deque<int64_t> &deque);
};

#endif // SSL_VISION_TIMESYNC_H
