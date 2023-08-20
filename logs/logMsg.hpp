#pragma once
#include <thread>
#include "util.hpp"
#include "level.hpp"

namespace Log
{
    struct LogMessage
    {
        using ptr = std::shared_ptr<LogMessage>;
        size_t _line;         // 行号
        size_t _ctime;        // 当前时间
        std::thread::id _tid; // 当前进程id
        std::string _file;    // 文件名
        std::string _name;    // 日志器名称
        std::string _payload; // 日志消息内容
        LogLevel::Level _lv;  // 日志等级

        LogMessage() {}
        LogMessage(
            LogLevel::Level lv,
            size_t line,
            std::string file,
            std::string name,
            std::string payload)
            : _line(line),
              _ctime(Util::Date::now()),
              _tid(std::this_thread::get_id()),
              _file(file),
              _name(name),
              _payload(payload),
              _lv(lv) {}
    };
}