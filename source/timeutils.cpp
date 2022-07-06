//
// Created by chen on 2022/7/4.
//
#include "timeutils.h"


#include "chrono"

namespace TIME{
    using namespace std::chrono;


      uint64_t getTimestampMs()
    {
        system_clock::duration d = system_clock::now().time_since_epoch();
        milliseconds mil = duration_cast<milliseconds>(d);
        return mil.count();
    }

      uint64_t getTimestampMic()
    {
        system_clock::duration d = system_clock::now().time_since_epoch();
        microseconds mic = duration_cast<microseconds>(d);
        return mic.count();
    }

      uint64_t getTimestampNan()
    {
        system_clock::duration d = system_clock::now().time_since_epoch();
        nanoseconds nan = duration_cast<nanoseconds>(d);
        return nan.count();
    }
}