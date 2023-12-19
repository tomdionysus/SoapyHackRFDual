//
// Soapy HackRF Dual Session Class
//
// Copyright (c) 2023 Tom Cully
//
#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

namespace SoapyHackRFDual {

struct TimedBuffer {
  std::chrono::time_point<std::chrono::high_resolution_clock> when;
  char* buffer;
  uint32_t length;
};

// Comparator for the priority_queue
struct CompareTimedBuffer {
  bool operator()(const TimedBuffer& a, const TimedBuffer& b) const;
};

class TimedBufferQueue {
 public:
  TimedBufferQueue();
  ~TimedBufferQueue();

  void addBuffer(
      std::chrono::time_point<std::chrono::high_resolution_clock> when,
      char* buffer, uint32_t length);
  void setCurrentTime(
      std::chrono::time_point<std::chrono::high_resolution_clock> now);

  void setLeadTime(
      std::chrono::duration<long long, std::nano> leadt);

  bool isEmpty();
  
  std::optional<TimedBuffer> getNextAfterSleep();

 private:
  std::priority_queue<TimedBuffer, std::vector<TimedBuffer>, CompareTimedBuffer>
      buffers;
  std::mutex buffersMutex;
  std::chrono::duration<long long, std::nano> offset;
  std::chrono::duration<long long, std::nano> leadTime;
};

}  // namespace SoapyHackRFDual
