//
// Soapy HackRF Dual Session Class
//
// Copyright (c) 2023 Tom Cully
//
#include "TimedBufferQueue.hpp"

namespace SoapyHackRFDual {

bool CompareTimedBuffer::operator()(const TimedBuffer& a,
                                    const TimedBuffer& b) const {
  return a.when > b.when;
}

TimedBufferQueue::TimedBufferQueue() : leadTime(std::chrono::microseconds(0)) {}

TimedBufferQueue::~TimedBufferQueue() {
  // Properly manage resources, if necessary
}

void TimedBufferQueue::addBuffer(
    std::chrono::time_point<std::chrono::high_resolution_clock> when,
    char* buffer, uint32_t length) {
  std::unique_lock<std::mutex> lock(buffersMutex);
  buffers.push(TimedBuffer{when, buffer, length});
}

void TimedBufferQueue::setCurrentTime(
    std::chrono::time_point<std::chrono::high_resolution_clock> now) {
  auto currentSystemTime = std::chrono::high_resolution_clock::now();
  offset = std::chrono::duration_cast<std::chrono::duration<long long, std::nano>>(currentSystemTime - now);

}

void TimedBufferQueue::setLeadTime(std::chrono::duration<long long, std::nano> leadt) {
  leadTime = leadt;
}

bool TimedBufferQueue::isEmpty() {
  std::unique_lock<std::mutex> lock(buffersMutex);
  return buffers.empty();
}

std::optional<TimedBuffer> TimedBufferQueue::getNextAfterSleep() {
  std::unique_lock<std::mutex> lock(buffersMutex);
  if (buffers.empty()) {
    return std::nullopt;  // Indicates an empty queue
  }

  auto nextTime = buffers.top().when - offset - leadTime;
  lock.unlock();  // Unlock before sleeping

  std::this_thread::sleep_until(nextTime);

  lock.lock();  // Re-lock before accessing the queue again
  TimedBuffer nextBuffer = buffers.top();
  buffers.pop();
  return nextBuffer;
}

}  // namespace SoapyHackRFDual
