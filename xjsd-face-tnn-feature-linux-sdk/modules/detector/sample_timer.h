#ifndef TNN_EXAMPLES_BASE_SAMPLE_TIMER_H_
#define TNN_EXAMPLES_BASE_SAMPLE_TIMER_H_

#include <chrono>
#include <string>

#include "tnn/core/macro.h"

namespace TNN_NS {

using std::chrono::time_point;
using std::chrono::system_clock;

class SampleTimer {
public:
    SampleTimer() {};
    void Start();
    void Stop();
    void Reset();
    double GetTime();

private:
    time_point<system_clock> start_;
    time_point<system_clock> stop_;
};

} // namespace TNN_NS

#endif // TNN_EXAMPLES_BASE_SAMPLE_TIMER_H_ 
