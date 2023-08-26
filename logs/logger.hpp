#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unordered_map>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <atomic>
#include "logMsg.hpp"
#include "level.hpp"
#include "formatter.hpp"
#include "util.hpp"
#include "out.hpp"
#include "async.hpp"

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

        std::string loggerName()
        {
            return _logger_name;
        }

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
            // std::cout << "res = " << res << std::endl;
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
            // std::cout << "serialize = "<<ss.str() << std::endl;
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
            // std::cout << "SyncLogger construction" << std::endl;
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

    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(const std::string &logger_name,
                    LogLevel::Level level,
                    Formatter::ptr pfmt,
                    std::vector<Output::ptr> outputs,
                    AsyncType looper_type = AsyncType::ASYNC_SAFE)
            : Logger(logger_name, level, pfmt, outputs),
              _plooper(std::make_shared<AsyncLooper>(std::bind(&AsyncLogger::realLog, this, std::placeholders::_1), looper_type))
        {
            // std::cout << "AsyncLogger construction" << std::endl;
        }

    protected:
        void log(const char *data, size_t len)
        {
            // 异步日志器的写入本质上是向生产者缓冲区中写入, 由异步线程将数据从缓冲区输出到指定位置
            // std::cout << "async log" << std::endl;
            // std::cout << data;
            _plooper->push(data, len);
            // std::cout << "push data: " << data << std::endl;
        }

        void realLog(Buffer &buff)
        {
            // 异步线程不需要上锁, 因为异步线程是单个执行流串行化执行, 不存在线程安全问题
            if (_outputs.empty())
            {
                return;
            }
            for (auto &out : _outputs)
            {
                // buff.getPtrPos();
                out->log(buff.begin(), buff.readableSize());
            }
        }

    private:
        AsyncLooper::ptr _plooper;
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
            : _logger_type(SYNC_LOGGER),
              _limit_level(LogLevel::Level::DEBUG),
              _async_type(AsyncType::ASYNC_SAFE)
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
        void buildUnsafeAsync()
        {
            _async_type = AsyncType::ASYNC_UNSAFE;
        }
        virtual Logger::ptr build() = 0;

    protected:
        LoggerType _logger_type;                   // 日志器类型
        std::string _logger_name;                  // 日志器名
        std::atomic<LogLevel::Level> _limit_level; // 控制日志输出等级
        Formatter::ptr _pfmt;                      // 格式化器
        std::vector<Output::ptr> _outputs;         // 存储输出器
        AsyncType _async_type;                     // 异步日志器类型
    };

    class LocalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(!_logger_name.empty()); // 保证日志器名称不为空
            if (_pfmt.get() == nullptr)
            {
                // 如果格式化器为空, 就创建一个格式化器
                _pfmt = std::make_shared<Formatter>();
            }
            if (_outputs.empty())
            {
                // 如果没有输出器, 默认添加一个标准输出
                buildOutputType<StdOutput>();
            }
            if (_logger_type == ASYNC_LOGGER)
            {
                // 如果是异步输出
                return std::make_shared<AsyncLogger>(_logger_name, _limit_level, _pfmt, _outputs, _async_type);
            }
            return std::make_shared<SyncLogger>(_logger_name, _limit_level, _pfmt, _outputs);
        }
    };

    // 日志器管理器, 当没有日志器需要管理时, 创建一个默认的管理器
    class LoggerManager
    {
        const std::string default_logger = "root";

    public:
        class GC
        {
        public:
            ~GC()
            {
                if (LoggerManager::_p_manager)
                {
                    delete LoggerManager::_p_manager;
                    //_p_manager = nullptr;
                }
            }
        };
        // 获取管理器实例
        static LoggerManager *getLoggerManager()
        {
            if (_p_manager == nullptr)
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_p_manager == nullptr)
                {
                    _p_manager = new LoggerManager();
                }
            }
            return _p_manager;
            // static LoggerManager manager;
            // return &manager;
        }
        // 判断是否存在某个日志器
        bool isExists(const std::string &logger_name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_loggers.count(logger_name))
                return true;
            return false;
        }
        // 添加日志器
        void addLogger(const Logger::ptr &logger)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _loggers[logger->loggerName()] = logger;
        }
        // 获取一个日志器
        Logger::ptr getLogger(const std::string &logger_name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_loggers.count(logger_name) == 0)
                return nullptr;
            return _loggers[logger_name];
        }
        // 获取默认日志器
        Logger::ptr getRootLogger()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _loggers[default_logger];
        }

        void print()
        {
            for (auto &it : _loggers)
            {
                std::cout << it.first << ":" << it.second << std::endl;
            }
        }

    private:
        LoggerManager()
        {
            // 先创建一个局部的日志器
            std::unique_ptr<LocalLoggerBuilder> builder(new LocalLoggerBuilder());
            builder->buildLoggerName(default_logger);
            _root_logger = builder->build();
            _loggers[_root_logger->loggerName()] = _root_logger;
        }
        LoggerManager(const LoggerManager &) = delete;
        LoggerManager operator=(const LoggerManager &) = delete;

    private:
        Logger::ptr _root_logger;                              // 默认日志器
        std::unordered_map<std::string, Logger::ptr> _loggers; // 日志器管理器
        static std::mutex _mutex;
        static LoggerManager *_p_manager;
    };
    LoggerManager *LoggerManager::_p_manager = nullptr;
    std::mutex LoggerManager::_mutex;
    LoggerManager::GC gc;

    class LoggerManager;
    class GlobalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build()
        {
            assert(!_logger_name.empty()); // 保证日志器名称不为空
            if (_pfmt.get() == nullptr)
            {
                // 如果格式化器为空, 就创建一个格式化器
                _pfmt = std::make_shared<Formatter>();
            }
            if (_outputs.empty())
            {
                // 如果没有输出器, 默认添加一个标准输出
                buildOutputType<StdOutput>();
            }
            Logger::ptr ret;
            if (_logger_type == ASYNC_LOGGER)
            {
                // 如果是异步输出
                ret = std::make_shared<AsyncLogger>(_logger_name, _limit_level, _pfmt, _outputs, _async_type);
            }
            else
            {
                // 同步输出
                ret = std::make_shared<SyncLogger>(_logger_name, _limit_level, _pfmt, _outputs);
            }
            // 全局的日志器只需要把日志器添加到管理器中, 即可在全局访问
            Log::LoggerManager::getLoggerManager()->addLogger(ret);
            return ret;
        }
    };
}