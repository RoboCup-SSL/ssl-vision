#include "TimeSync.h"

const int BUFFER_SIZE = 30;
const uint64_t MAX_AVG_DIFF = 3e8L;  // 300ms
const uint64_t SYNC_ACCURACY = 1e6L; // 1ms

TimeSync::TimeSync() { currentOffset = 0; }

void TimeSync::update(uint64_t timestamp) {
  timeval tv = {};
  gettimeofday(&tv, nullptr);
  uint64_t tRef = tv.tv_sec * 1e9L + tv.tv_usec * 1e3L;
  uint64_t tSynced = timestamp - currentOffset;

  int64_t diff = tRef - tSynced;
  diffBuffer.push_back(diff);
  if (diffBuffer.size() > BUFFER_SIZE)
    diffBuffer.pop_front();
  uint64_t avgDiff = labs(average(diffBuffer));

  if ((avgDiff > MAX_AVG_DIFF) || !offsetBuffer.empty()) {
    // Start syncing
    if (offsetBuffer.empty()) {
      std::cout << "Start syncing with system clock due to avgDiff=" << avgDiff << std::endl;
    }
    int64_t offset = timestamp - tRef;
    offsetBuffer.push_back(offset);
    if (offsetBuffer.size() > BUFFER_SIZE)
      offsetBuffer.pop_front();
    currentOffset = average(offsetBuffer);
    if (avgDiff < SYNC_ACCURACY) {
      // Converged, stop syncing with reference clock
      std::cout << "Synced with system clock. offset=" << currentOffset
                << "ns diff=" << avgDiff << "ns" << std::endl;
      offsetBuffer.clear();
    }
  }
}

uint64_t TimeSync::sync(uint64_t timestamp) const {
  return timestamp - currentOffset;
}

int64_t TimeSync::calcOffset(uint64_t tRef, uint64_t tOther) {
  return tOther - tRef;
}

int64_t TimeSync::average(const std::deque<int64_t> &deque) {
  int size = deque.size();
  double avg = 0;
  for (auto l : deque) {
    avg += (double)l / size;
  }
  return avg;
}