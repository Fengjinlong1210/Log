#pragma once
#include "logger.hpp"

namespace Log
{
    Logger::ptr getLogger(const std::string& logger_name)
    {
        return LoggerManager::getLoggerManager()->getLogger(logger_name);
    }

    Logger::ptr rootLogger()
    {
        return LoggerManager::getLoggerManager()->getRootLogger();
    }   

    #define debug(fmt, ...) debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define info(fmt, ...) info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define warning(fmt, ...) warning(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define error(fmt, ...) error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define fatal(fmt, ...) fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

    #define DEBUG(fmt, ...) Log::rootLogger()->debug(fmt, ##__VA_ARGS__)
    #define INFO(fmt, ...) Log::rootLogger()->info(fmt, ##__VA_ARGS__)
    #define WARNING(fmt, ...) Log::rootLogger()->warning(fmt, ##__VA_ARGS__)
    #define ERROR(fmt, ...) Log::rootLogger()->error(fmt, ##__VA_ARGS__)
    #define FATAL(fmt, ...) Log::rootLogger()->fatal(fmt, ##__VA_ARGS__)
    
}