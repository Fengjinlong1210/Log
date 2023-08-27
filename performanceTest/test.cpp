#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "../logs/log.hpp"
using namespace Log;
void performanceTest(const std::string &logger_name, size_t thread_cnt, size_t msg_cnt, size_t msg_size)
{
    std::cout << "日志器名: " << logger_name << std::endl;
    std::cout << "线程个数: " << thread_cnt << std::endl;
    std::cout << "日志个数: " << msg_cnt << std::endl;
    Logger::ptr logger = LoggerManager::getLoggerManager()->getLogger(logger_name);
    if (logger.get() == nullptr)
        return;
    std::string msg(msg_size, 'a');
    size_t msg_cnt_thread = msg_cnt / thread_cnt;
    std::vector<std::thread> threads;
    double max_time;
    for (int i = 0; i < thread_cnt; ++i)
    {
        threads.emplace_back([&, i]
                             {
            auto start = std::chrono::high_resolution_clock::now();
            for(int j = 0; j < msg_cnt_thread; ++j)
            {
                logger->debug("%s", msg.c_str());
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto time_gap = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
            if(time_gap.count() > max_time) max_time = time_gap.count();
            auto avg = msg_cnt_thread / time_gap.count();
            std::cout << "线程"<<i<<"打印"<<msg_cnt_thread<<"条日志, 耗时" <<time_gap.count()<<"s, 平均"<<avg<<"条/s" << std::endl; });
    }
    for (int i = 0; i < thread_cnt; ++i)
    {
        threads[i].join();
    }
    std::cout << "\t最大耗时: " << max_time << "s" << std::endl;
    std::cout << "\t平均每秒输出: " << (size_t)msg_cnt / max_time << " 条日志" << std::endl;
    std::cout << "\t平均每秒输出: " << (size_t)(msg_cnt * msg_size / max_time / 1024) << " kb" << std::endl;
}

void testSync()
{
    std::cout << "--------------------------------------------------" << std::endl;
    INFO("%s", "同步日志性能测试");
    std::string logger_name = "SyncLogger";
    std::unique_ptr<GlobalLoggerBuilder> builder(new GlobalLoggerBuilder());
    builder->buildLoggerType(LoggerType::SYNC_LOGGER);
    builder->buildLoggerName(logger_name);
    builder->buildOutputType<FileOutput>("./fileout/file.log");
    builder->buildOutputType<RollOutput>("./rollout/roll.log", 1024 * 1024);
    auto logger = builder->build();
    performanceTest(logger_name, 5, 2000000, 20);
    INFO("%s", "同步日志性能测试结束");
    std::cout << "--------------------------------------------------" << std::endl;
}

void testAsync()
{
    std::cout << "--------------------------------------------------" << std::endl;
    INFO("%s", "异步日志性能测试");
    std::string logger_name = "AsyncLogger";
    std::unique_ptr<GlobalLoggerBuilder> builder(new GlobalLoggerBuilder());
    builder->buildLoggerType(LoggerType::ASYNC_LOGGER);
    builder->buildLoggerName(logger_name);
    builder->buildOutputType<FileOutput>("./fileout/file.log");
    builder->buildOutputType<RollOutput>("./rollout/roll.log", 1024 * 1024);
    auto logger = builder->build();
    performanceTest(logger_name, 5, 2000000, 20);
    INFO("%s", "异步日志性能测试结束");
    std::cout << "--------------------------------------------------" << std::endl;
}

int main()
{
    testSync();
    //testAsync();
    return 0;
}