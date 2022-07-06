//
// Created by chen on 2022/6/30.
//

#ifndef UNTITLED_TIMEUTILS_H
#define UNTITLED_TIMEUTILS_H
#include "chrono"

namespace TIME{
    using namespace std::chrono;

     uint64_t getTimestampMs();

     uint64_t getTimestampMic();

     uint64_t getTimestampNan();
}

#endif //UNTITLED_TIMEUTILS_H
