#include "TimedBufferQueue.hpp"  // Include your TimedBufferQueue header

#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <sstream>
#include <ctime>
#include <climits>

#include <pthread.h>

#define TOTAL_TEST_TIME_US 5000000
#define TEST_ITERATIONS 10000
#define LEAD_TIME_US -1342

void setHighPriority() {
    pthread_t this_thread = pthread_self();
    
    struct sched_param params;
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    
    pthread_setschedparam(this_thread, SCHED_FIFO, &params);
}

// Function to convert time_point to human-readable format
std::string formatTime(std::chrono::time_point<std::chrono::high_resolution_clock> timePoint) {
    // Convert time_point to system_clock time_point
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::time_point_cast<std::chrono::microseconds>(now) - 
                  std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::from_time_t(now_c));
    
    // Convert to local time
    std::tm localTime = *std::localtime(&now_c);

    // Format time
    std::stringstream ss;
    ss << std::put_time(&localTime, "%H:%M:%S") << '.' 
       << std::setfill('0') << std::setw(6) << now_ms.count();

    return ss.str();
}

int main() {

    setHighPriority();

    SoapyHackRFDual::TimedBufferQueue runner;

    runner.setLeadTime(std::chrono::microseconds(LEAD_TIME_US));

    // Random number generator for buffer times
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, TOTAL_TEST_TIME_US);  // Time range in microseconds for 5 seconds

    // Current time
    auto now = std::chrono::high_resolution_clock::now();

    // Load mock buffers
    for (int i = 0; i < TEST_ITERATIONS; ++i) {  // 1000 buffers
        auto randomTimeOffset = std::chrono::microseconds(dis(gen));
        auto bufferTime = now + randomTimeOffset;
        char* mockBuffer = nullptr;  // Mock buffer

        runner.addBuffer(bufferTime, mockBuffer, 0);  // Length is 0 for mock buffer
        if (i % 100 == 0) {
            std::cout << "\rBuffer " << i << " time added: " << formatTime(bufferTime) << std::flush;
        }
    }
    std::cout << std::endl;

    // Set the current time in runner
    runner.setCurrentTime(now);

    std::cout << "\rCurrent time before starting: " << formatTime(now) << std::endl;

    long long totalOffset = 0;
    long long minOffset = LLONG_MAX;
    long long maxOffset = 0;
    int bufferCount = 0;

    // Loop until the queue is empty
    while (!runner.isEmpty()) {
        auto nextBuffer = runner.getNextAfterSleep();
        if (nextBuffer) {
            auto actualTime = std::chrono::high_resolution_clock::now();
            auto offset = std::chrono::duration_cast<std::chrono::microseconds>(actualTime - nextBuffer->when).count();
            totalOffset += offset;
            minOffset = std::min(minOffset, offset);
            maxOffset = std::max(maxOffset, offset);

            bufferCount++;

            if (bufferCount % 500 == 0) {
                std::cout << "\rBuffer expected time: " << formatTime(nextBuffer->when)
                          << ", Actual time: " << formatTime(actualTime)
                          << ", Offset: " << offset << " µs" << std::flush;
            }
        }
    }

    if (bufferCount > 0) {
        std::cout << "\nJitter (Average offset): Min/Max/Avg: " << minOffset << " / " << maxOffset << " / " << (totalOffset / bufferCount) << " µs" << std::endl;
        std::cout << "Configured Correction: " << LEAD_TIME_US << "µs" << std::endl;
        std::cout << "Recommended Correction: " << 0-((maxOffset - minOffset) /2) << "µs" << std::endl;

    }

    return 0;
}
