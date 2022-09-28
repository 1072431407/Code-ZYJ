#include "sample_timer.h"

#include <cmath>

namespace TNN_NS {

using std::chrono::duration_cast;
using std::chrono::microseconds;

void SampleTimer::Start() {
    start_ = system_clock::now();
}

void SampleTimer::Stop() {
    stop_ = system_clock::now();
}

double SampleTimer::GetTime() {
    return duration_cast<microseconds>(stop_ - start_).count() / 1000.0f;
}

void SampleTimer::Reset() {
    stop_ = start_ = system_clock::now();
}

}
