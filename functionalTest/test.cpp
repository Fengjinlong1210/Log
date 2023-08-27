#include <unistd.h>
#include "util.hpp"
#include "level.hpp"
#include "formatter.hpp"
#include "out.hpp"
#include "logger.hpp"
#include "buffer.hpp"
#include "log.hpp"
using namespace std;
using namespace Log;

void testUtil()
{
    cout << Log::Util::Date::now() << endl;
    string filepath = "./abc/bcd/def/a.txt";
    string path = Util::File::path(filepath);
    cout << path << endl;
    Util::File::create_directory(path);
}

void testLevel()
{
    cout << Log::LogLevel::levelToStr(LogLevel::DEBUG) << endl;
    cout << Log::LogLevel::levelToStr(LogLevel::ERROR) << endl;
    cout << Log::LogLevel::levelToStr(LogLevel::FATAL) << endl;
    cout << Log::LogLevel::levelToStr(LogLevel::INFO) << endl;
    cout << Log::LogLevel::levelToStr(LogLevel::OFF) << endl;
    cout << Log::LogLevel::levelToStr(LogLevel::UNKNOW) << endl;
    cout << Log::LogLevel::levelToStr(LogLevel::WARNING) << endl;
}

void testFormat()
{
    Log::LogMessage msg(LogLevel::INFO, 150, "test.cpp", "root", "测试输出接口");
    Log::Formatter fmtr;
    std::string ret = fmtr.format(msg);
    std::cout << ret;
}

// void testOut()
// {
//     Log::LogMessage msg(LogLevel::INFO, 150, "test.cpp", "root", "测试输出接口");
//     Log::Formatter fmtr;
//     std::string ret = fmtr.format(msg);
//     // Log::StdOutput stdOut;
//     // Log::FileOutput fOut("./logfile/info.log");
//     // Log::RollOutput rOut("./rollfile/info.log", 1024 * 1024);
//     // stdOut.log(ret.c_str(), ret.size());
//     // fOut.log(ret.c_str(), ret.size());
//     // size_t cursize = 0;
//     // size_t cnt = 0;
//     // while(cursize < 1024 * 1024 * 10)
//     // {
//     //     std::string tmp = to_string(cnt) + ":" + ret;
//     //     rOut.log(tmp.c_str(), tmp.size());
//     //     cursize += tmp.size();
//     //     cnt++;
//     // }
//     Log::Output::ptr stdOut_ptr = Log::OutputFactory::create<Log::StdOutput>();
//     Log::Output::ptr fileOut_ptr = Log::OutputFactory::create<Log::FileOutput>("./logfile/info.log");
//     Log::Output::ptr rollOut_ptr = Log::OutputFactory::create<Log::RollOutput>("./rollfile/info.log", 1024 * 1024);
//     stdOut_ptr->log(ret.c_str(), ret.size());
//     fileOut_ptr->log(ret.c_str(), ret.size());
//     size_t cursize = 0;
//     size_t cnt = 0;
//     while (cursize < 1024 * 1024 * 10)
//     {
//         string tmp = to_string(cnt++) + ":" + ret;
//         rollOut_ptr->log(tmp.c_str(), tmp.size());
//         cursize += tmp.size();
//     }
// }

// void testOut2()
// {
//     auto ptr = OutputFactory::create<TimeRollOutput>("./timeroll/time.log", Min);
//     Log::LogMessage msg(LogLevel::INFO, 150, "test.cpp", "root", "测试输出接口");
//     Log::Formatter fmtr;
//     std::string ret = fmtr.format(msg);
//     time_t flag = Util::Date::now();
//     size_t cnt = 0;
//     while (Util::Date::now() < flag + 60 * 5)
//     {
//         string tmp = to_string(cnt++) + ":" + ret;
//         ptr->log(tmp.c_str(), tmp.size());
//         sleep(1);
//     }
// }

void testSync()
{
    std::string logger_name = "SyncLogger";
    LogLevel::Level limit = LogLevel::Level::WARNING;
    Formatter::ptr fmt(new Formatter());
    Output::ptr p_std = OutputFactory::create<StdOutput>();
    Output::ptr p_file = OutputFactory::create<FileOutput>("./logfile/test.log");
    Output::ptr p_roll = OutputFactory::create<RollOutput>("./rollfile/roll-", 1024 * 1024);
    vector<Output::ptr> out_arr = {p_std, p_file, p_roll};
    // vector<Output::ptr> out_arr = {p_std, p_file};
    SyncLogger synclogger(logger_name, limit, fmt, out_arr);

    synclogger.debug(__FILE__, __LINE__, "%s", "测试同步日志器");
    synclogger.info(__FILE__, __LINE__, "%s", "测试同步日志器");
    synclogger.warning(__FILE__, __LINE__, "%s", "测试同步日志器");
    synclogger.error(__FILE__, __LINE__, "%s", "测试同步日志器");
    synclogger.fatal(__FILE__, __LINE__, "%s", "测试同步日志器");

    size_t cur_size = 0;
    size_t count = 0;
    while (cur_size < 1024 * 1024 * 10)
    {
        synclogger.debug(__FILE__, __LINE__, "%d - %s", count, "测试同步日志器-滚动");
        synclogger.fatal(__FILE__, __LINE__, "%d - %s", count++, "测试同步日志器-滚动");
        cur_size += 50;
    }
}

void testBulider()
{
    std::unique_ptr<LoggerBuilder> p_builder(new LocalLoggerBuilder());
    p_builder->buildLoggerName("Test");
    p_builder->buildLoggerLevel(Log::LogLevel::Level::DEBUG);
    p_builder->buildLoggerType(LoggerType::SYNC_LOGGER);
    p_builder->buildFormatter();
    p_builder->buildOutputType<Log::FileOutput>("./logfile/test.log");
    Log::Logger::ptr p_logger = p_builder->build();
    size_t cur_size = 0;
    size_t cnt = 0;
    while (cur_size < 1024 * 1024)
    {
        p_logger->fatal(__FILE__, __LINE__, "%d-%s", cnt++, "测试建造者");
        cur_size += 40;
    }
}

void testBuffer()
{
    // 从文件中读取数据到buffer, 再从buffer读取到字符串中
    ifstream ifs;
    ifs.open("./logfile/test.log", ios::binary);
    if (!ifs.is_open())
    {
        cout << "read from file error" << endl;
    }

    ifs.seekg(0, ios::end);
    size_t filesize = ifs.tellg(); // 获取文件大小
    ifs.seekg(0, ios::beg);
    string body;
    body.resize(filesize);
    ifs.read(&body[0], filesize);
    if (!ifs.good())
    {
        cout << "ifs read error" << endl;
    }
    ifs.close();
    cout << filesize << endl;
    cout << body.size() << endl;
    Buffer buffer;
    cout << "after push" << endl;
    for (size_t i = 0; i < body.size(); ++i)
    {
        buffer.push(&body[i], 1);
        // buffer.moveWriter(1);
    }
    // buffer.push(&body[0], filesize);
    cout << "push error" << endl;
    ofstream ofs;
    ofs.open("./logfile/test2.log", ios::binary);
    if (!ofs.is_open())
    {
        cout << "open error" << endl;
        return;
    }
    ofs.write(buffer.begin(), buffer.readableSize());
    if (!ofs.good())
    {
        cout << "ofs write error" << endl;
        return;
    }
    ofs.close();
}

void testAsync()
{
    std::shared_ptr<LocalLoggerBuilder> builder(new LocalLoggerBuilder());
    builder->buildLoggerName("ASYNC logger");
    builder->buildLoggerLevel(LogLevel::Level::DEBUG);
    builder->buildLoggerType(LoggerType::ASYNC_LOGGER);
    builder->buildFormatter();
    // builder->buildOutputType<StdOutput>();
    builder->buildOutputType<FileOutput>("./logfile/async.log");
    auto async_logger = builder->build();
    for (int i = 0; i < 10000; i++)
    {
        async_logger->debug(__FILE__, __LINE__, "%d-%s", i + 1, "测试异步日志器");
    }
    // async_logger->debug(__FILE__, __LINE__, "%d-%s", 1, "测试异步日志器");
    // async_logger->info(__FILE__, __LINE__, "%d-%s", 1, "测试异步日志器");
    // async_logger->error(__FILE__, __LINE__, "%d-%s", 1, "测试异步日志器");
    // async_logger->warning(__FILE__, __LINE__, "%d-%s", 1, "测试异步日志器");
    // async_logger->fatal(__FILE__, __LINE__, "%d-%s", 1, "测试异步日志器");

    ifstream ifs;
    ifs.open("./logfile/async.log");
    if (!ifs.is_open())
    {
        cout << "open error" << endl;
        return;
    }
    ifs.seekg(0, ios::end);
    size_t len = ifs.tellg();
    ifs.seekg(0, ios::beg);
    cout << "read len = " << len << endl;
    string body;
    ifs.read(&body[0], len);
    if (!ifs.good())
    {
        cout << "read error" << endl;
        return;
    }
    cout << body << endl;
}

void testManager()
{
    string logger_name = "test_logger";
    std::shared_ptr<LoggerBuilder> builder(new LocalLoggerBuilder());
    builder->buildFormatter();
    builder->buildLoggerLevel(LogLevel::Level::WARNING);
    builder->buildLoggerName(logger_name);
    builder->buildLoggerType(LoggerType::ASYNC_LOGGER);
    builder->buildOutputType<FileOutput>("./logfile/async.log");
    auto logger = builder->build();
    // 获取日志管理器
    LoggerManager *mngr = LoggerManager::getLoggerManager();
    // 添加日志器
    mngr->addLogger(logger);
    // 获取日志器
    Logger::ptr async_logger = mngr->getLogger(logger_name);
    Logger::ptr p_root = mngr->getRootLogger();
    // p_root->debug(__FILE__, __LINE__, "%s", "test manager");
    for (int i = 0; i < 1000; ++i)
    {
        async_logger->debug(__FILE__, __LINE__, "%s-%d", "asyncLogger", i + 1);
        async_logger->info(__FILE__, __LINE__, "%s-%d", "asyncLogger", i + 1);
        async_logger->warning(__FILE__, __LINE__, "%s-%d", "asyncLogger", i + 1);
        async_logger->error(__FILE__, __LINE__, "%s-%d", "asyncLogger", i + 1);
        async_logger->fatal(__FILE__, __LINE__, "%s-%d", "asyncLogger", i + 1);
    }
}

void testGlobal()
{
    shared_ptr<LoggerBuilder> builder(new GlobalLoggerBuilder());
    builder->buildFormatter();
    builder->buildLoggerLevel(LogLevel::Level::DEBUG);
    builder->buildLoggerName("global");
    builder->buildLoggerType(LoggerType::ASYNC_LOGGER);
    builder->buildUnsafeAsync();
    // builder->buildOutputType<StdOutput>();
    builder->buildOutputType<FileOutput>("./filelog/test.log");
    auto ptr = builder->build();
    // LoggerManager::getLoggerManager()->addLogger("global", ptr);
}

void testGlobal2()
{
    auto ptr = LoggerManager::getLoggerManager()->getLogger("global");
    for (int i = 0; i < 1000; ++i)
    {
        ptr->debug(__FILE__, __LINE__, "%s-%d", "global logger", i + 1);
        // ptr->info(__FILE__, __LINE__, "%s-%d", "global logger", i + 1);
        // ptr->warning(__FILE__, __LINE__, "%s-%d", "global logger", i + 1);
        // ptr->error(__FILE__, __LINE__, "%s-%d", "global logger", i + 1);
        // ptr->fatal(__FILE__, __LINE__, "%s-%d", "global logger", i + 1);
    }
    LoggerManager::getLoggerManager()->print();
}

void testAsync2()
{
    std::shared_ptr<LoggerBuilder> builder(new LocalLoggerBuilder());
    builder->buildLoggerName("ASYNC logger");
    builder->buildLoggerLevel(LogLevel::Level::DEBUG);
    builder->buildLoggerType(LoggerType::ASYNC_LOGGER);
    builder->buildFormatter();
    builder->buildUnsafeAsync();
    // builder->buildOutputType<StdOutput>();
    builder->buildOutputType<FileOutput>("./logfile/async.log");
    auto async_logger = builder->build();
    for (int i = 0; i < 10000; i++)
    {
        async_logger->debug(__FILE__, __LINE__, "%d-%s", i + 1, "测试异步日志器");
    }
}

void testMacro()
{
    //DEBUG("%s", "测试");
    string logger_name = "ASYNC LOGGER";
    shared_ptr<GlobalLoggerBuilder> builder(new GlobalLoggerBuilder());
    builder->buildLoggerType(LoggerType::ASYNC_LOGGER);
    builder->buildOutputType<FileOutput>("./logfile/async.log");
    //builder->buildOutputType<StdOutput>();
    builder->buildLoggerName(logger_name);
    auto manager = Log::LoggerManager::getLoggerManager();
    manager->addLogger(builder->build());
    Logger::ptr lgr = getLogger(logger_name);
    for(int i = 0; i < 1000; ++i)
    {
        lgr->debug("%s-%d", "测试", i + 1);
        //lgr->info("%s-%d", "测试", i + 1);
        //lgr->warning("%s-%d", "测试", i + 1);
        //lgr->error("%s-%d", "测试", i + 1);
        //lgr->fatal("%s-%d", "测试", i + 1);
    }
}

int main()
{
    // testLevel();
    // testFormat();
    // testOut2();
    // testSync();
    // testBulider();
    // testBuffer();
    // testAsync();
    // testManager();
    //testGlobal();
    //testGlobal2();
    // int cnt = 2;
    // while(cnt--)
    // {
    //     ;
    // }
    //testAsync2();
    testMacro();
    //sleep(2);
    //LoggerManager::getLoggerManager()->~LoggerManager();
    return 0;
}


// class singleton
// {
// public:
// 	static singleton* get_instance()
// 	{
// 		if (!_only_instance)
// 		{
// 			_mtx.lock();		//为了保证线程安全, 必须双层检验
// 			if (!_only_instance)
// 			{
// 				_only_instance = new singleton;
// 			}
// 			_mtx.unlock();
// 		}
// 		return _only_instance;
// 	}
// private:
// 	singleton() {}	//构造函数私有, 防止类外创建对象
// 	//禁止生成拷贝构造和赋值
// 	singleton(const singleton&) = delete;
// 	singleton& operator=(const singleton&) = delete;

// private:
// 	static singleton* _only_instance;
// 	static std::mutex _mtx;
// };
// singleton* singleton::_only_instance = nullptr;
// std::mutex singleton::_mtx;

// class Singleton
// {
// public:
//     static Singleton* get_instance()
//     {
//         //C++11之后, 多个线程同时试图同时初始化一个静态变量, 该变量只会被初始化一次
//         //也就是保证静态变量初始化时的线程安全
//         static Singleton _instance;
//         return &_instance;
//     }

// private:
//     Singleton(){}
//     ~Singleton(){}
//     Singleton(const Singleton&) = delete;
// };