#ifndef ALOGGER_H
#define ALOGGER_H

#include "noncopyable.h"
#include "mylog/MyLog.hpp"

#include <string>

class ALogger: noncopyable
{
public:
    ALogger();
    ~ALogger() = default;

    void DEBUG(const std::string& unformatted_message);
    void INFO(const std::string& unformatted_message);
    void WARN(const std::string& unformatted_message);
    void ERROR(const std::string& unformatted_message);
    void FATAL(const std::string& unformatted_message);

private:
    mylog::AsyncLogger::ptr logPtr_;
};



extern ALogger* logger_;

#endif