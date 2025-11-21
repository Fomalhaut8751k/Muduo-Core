#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "copyable.h"

#include <iostream>
#include <string>

class TimeStamp: copyable
{
private:
    int64_t microSecondsSinceEpoch_;
public:
    TimeStamp();
    explicit TimeStamp(int64_t microSecondsSinceEpoch);

    static TimeStamp now(); 
    std::string toString() const;
};

#endif