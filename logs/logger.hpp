#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <atomic>
#include "logMsg.hpp"
#include "level.hpp"
#include "formatter.hpp"
#include "util.hpp"
#include "out.hpp"

namespace Log
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name,
               LogLevel::Level level,
               Formatter::ptr pfmt,
               std::vector<Output::ptr> outputs)
            : _logger_name(logger_name),
              _limit_level(level),
              _pfmt(pfmt),
              _outputs(outputs) {}

        // 构造日志消息对象, 对日志消息进行格式化, 输出字符串, 然后进行落地输出
        void debug(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            // 1. 判断输出等级是否满足, 不满足就返回
            if (_limit_level > LogLevel::Level::DEBUG)
            {
                return;
            }
            // 2. 对fmt和不定参函数进行解析, 形成字符串
            char *res;
            va_list p; // 不定参指针
            va_start(p, fmt);
            int ret = vasprintf(&res, fmt.c_str(), p);
            std::cout << res << std::endl;
            if (ret == -1)
            {
                std::cout << "vasprintf error" << std::endl;
                return;
            }
            va_end(p);
            serialize(LogLevel::Level::DEBUG, file, line, res);
            free(res);
        }
        void info(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            // 1. 判断输出等级是否满足, 不满足就返回
            if (_limit_level > LogLevel::Level::INFO)
            {
                return;
            }
            // 2. 对fmt和不定参函数进行解析, 形成字符串
            char *res;
            va_list p; // 不定参指针
            va_start(p, fmt);
            int ret = vasprintf(&res, fmt.c_str(), p);
            if (ret == -1)
            {
                std::cout << "vasprintf error" << std::endl;
                return;
            }
            va_end(p);
            serialize(LogLevel::Level::INFO, file, line, res);
            free(res);
        }
        void warning(const std::string &file, size_t line, const std::string &fmt, ...)
        { // 1. 判断输出等级是否满足, 不满足就返回
            if (_limit_level > LogLevel::Level::WARNING)
            {
                return;
            }
            // 2. 对fmt和不定参函数进行解析, 形成字符串
            char *res;
            va_list p; // 不定参指针
            va_start(p, fmt);
            int ret = vasprintf(&res, fmt.c_str(), p);
            if (ret == -1)
            {
                std::cout << "vasprintf error" << std::endl;
                return;
            }
            va_end(p);
            serialize(LogLevel::Level::WARNING, file, line, res);
            free(res);
        }
        void error(const std::string &file, size_t line, const std::string &fmt, ...)
        { // 1. 判断输出等级是否满足, 不满足就返回
            if (_limit_level > LogLevel::Level::ERROR)
            {
                return;
            }
            // 2. 对fmt和不定参函数进行解析, 形成字符串
            char *res;
            va_list p; // 不定参指针
            va_start(p, fmt);
            int ret = vasprintf(&res, fmt.c_str(), p);
            if (ret == -1)
            {
                std::cout << "vasprintf error" << std::endl;
                return;
            }
            va_end(p);
            serialize(LogLevel::Level::ERROR, file, line, res);
            free(res);
        }
        void fatal(const std::string &file, size_t line, const std::string &fmt, ...)
        { // 1. 判断输出等级是否满足, 不满足就返回
            if (_limit_level > LogLevel::Level::FATAL)
            {
                return;
            }
            // 2. 对fmt和不定参函数进行解析, 形成字符串
            char *res;
            va_list p; // 不定参指针
            va_start(p, fmt);
            int ret = vasprintf(&res, fmt.c_str(), p);
            if (ret == -1)
            {
                std::cout << "vasprintf error" << std::endl;
                return;
            }
            va_end(p);
            serialize(LogLevel::Level::FATAL, file, line, res);
            free(res);
        }

    protected:
        virtual void log(const char *data, size_t len) = 0;
        void serialize(LogLevel::Level level, const std::string &file, size_t line, const char *str)
        {
            // 3. 构建logMsg对象
            LogMessage msg(level, line, file, _logger_name, str);
            // 4. 对logMsg进行格式化
            std::stringstream ss;
            _pfmt->format(ss, msg);
            // 5. 对格式化后的内容进行输出
            log(ss.str().c_str(), ss.str().size());
        }

    protected:
        std::mutex _mutex;                         // 互斥锁
        std::string _logger_name;                  // 日志器名
        std::atomic<LogLevel::Level> _limit_level; // 控制日志输出等级
        Formatter::ptr _pfmt;                      // 格式化器
        std::vector<Output::ptr> _outputs;         // 存储输出器
    };

    class SyncLogger : public Logger
    {
    public:
        SyncLogger(const std::string &logger_name,
                   LogLevel::Level level,
                   Formatter::ptr pfmt,
                   std::vector<Output::ptr> outputs)
            : Logger(logger_name, level, pfmt, outputs)
        {
            std::cout << "SyncLogger construction" << std::endl;
        }

    protected:
        void log(const char *data, size_t len)
        {
            // 释放时会自动解锁
            std::unique_lock<std::mutex> _lock(_mutex);
            if (_outputs.empty())
            {
                return;
            }
            for (auto &out : _outputs)
            {
                out->log(data, len);
            }
        }
    };

    enum LoggerType // 日志器类型
    {
        SYNC_LOGGER, // 同步日志器
        ASYNC_LOGGER // 异步日志器
    };
    // 日志器构造器, 创建日志器的零部件, 构造出日志器
    class LoggerBuilder
    {
    public:
        LoggerBuilder()
            : _logger_type(SYNC_LOGGER), _limit_level(LogLevel::Level::DEBUG)
        {
        }
        void buildLoggerType(LoggerType type) // 创建日志器类型(同步/异步)
        {
            _logger_type = type;
        }
        void buildLoggerName(const std::string &name) // 创建日志器名
        {
            _logger_name = name;
        }
        void buildLoggerLevel(LogLevel::Level level) // 创建日志器等级
        {
            _limit_level = level;
        }
        void buildFormatter(const std::string &pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n") // 创建格式化器
        {
            _pfmt = std::make_shared<Formatter>(pattern);
        }
        template <class OutputType, class... Args>
        void buildOutputType(Args &&...args) // 创建输出模式
        {
            Output::ptr p_out = OutputFactory::create<OutputType>(std::forward<Args>(args)...);
            _outputs.push_back(p_out);
        }

        virtual Logger::ptr build() = 0;

    protected:
        LoggerType _logger_type;                   // 日志器类型
        std::string _logger_name;                  // 日志器名
        std::atomic<LogLevel::Level> _limit_level; // 控制日志输出等级
        Formatter::ptr _pfmt;                      // 格式化器
        std::vector<Output::ptr> _outputs;         // 存储输出器
    };

    class LocalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(!_logger_name.empty());  //保证日志器名称不为空
            if (_pfmt.get() == nullptr)
            {
                //如果格式化器为空, 就创建一个格式化器
                _pfmt = std::make_shared<Formatter>();
            }
            if(_outputs.empty())
            {
                //如果没有输出器, 默认添加一个标准输出
                buildOutputType<StdOutput>();
            }
            if(_logger_type == ASYNC_LOGGER)
            {
                //如果是异步输出
            }
            return std::make_shared<SyncLogger>(_logger_name, _limit_level, _pfmt, _outputs);
        }

    protected:
    };
}