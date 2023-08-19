#pragma once
#include <iostream>

namespace Log
{
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOW = 0,
            DEBUG,
            INFO,
            WARNING,
            ERROR,
            FATAL,
            OFF
        };

        static const char *levelToStr(Log::LogLevel::Level lv)
        {
            switch (lv)
            {
            case DEBUG:
                return "DEBUG";
                break;
            case INFO:
                return "INFO";
                break;
            case WARNING:
                return "WARNING";
                break;
            case ERROR:
                return "ERROR";
                break;
            case FATAL:
                return "FATAL";
                break;
            case OFF:
                return "OFF";
                break;
            }
            return "UNKNOW";
        }
    };
}