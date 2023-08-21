#pragma once
#include <cassert>
#include <memory>
#include <mutex>
#include <fstream>
#include <sstream>
#include "util.hpp"

namespace Log
{
    class Output
    {
    public:
        using ptr = std::shared_ptr<Output>;
        Output() {}
        virtual ~Output() {}
        virtual void log(const char *data, size_t len) = 0;
    };

    class StdOutput : public Output
    {
    public:
        using ptr = std::shared_ptr<StdOutput>;

        void log(const char *data, size_t len)
        {
            std::cout.write(data, len);
        }
    };

    class FileOutput : public Output
    {
    public:
        using ptr = std::shared_ptr<FileOutput>;
        FileOutput(const std::string &pathname)
            : _pathname(pathname)
        {
            // 如果路径不存在就创建路径, 然后打开文件
            Util::File::create_directory(Util::File::path(_pathname));
            _ofs.open(_pathname, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        }

        void log(const char *data, size_t len)
        {
            _ofs.write(data, len);
            if (!_ofs.good())
            {
                std::cout << "文件输出失败" << std::endl;
            }
        }

    private:
        std::string _pathname;
        std::ofstream _ofs;
    };

    class RollOutput : public Output
    {
    public:
        using ptr = std::shared_ptr<RollOutput>;
        RollOutput(const std::string &basename, size_t max_size)
            : _basename(basename), _max_size(max_size), _cur_size(0)
        {
            // 如果路径不存在就创建路径, 然后打开文件
            Util::File::create_directory(Util::File::path(basename));
        }

        void log(const char *data, size_t len)
        {
            if (!_ofs.is_open() || _cur_size > _max_size)
            {
                // 当前没有文件打开或大小超出限制
                _ofs.close();
                std::string name = createNewFileName();
                _ofs.open(name, std::ios::binary | std::ios::app);
                assert(_ofs.is_open());
                _cur_size = 0;
            }
            _ofs.write(data, len);
            if (!_ofs.good())
            {
                std::cout << "写入滚动文件失败" << std::endl;
            }
            _cur_size += len;
        }

    private:
        std::string createNewFileName()
        {
            time_t t = time(nullptr);
            struct tm s_t;
            localtime_r(&t, &s_t);
            std::stringstream ss;
            ss << _basename;
            ss << s_t.tm_year + 1900;
            ss << s_t.tm_mon + 1;
            ss << s_t.tm_wday;
            ss << s_t.tm_hour;
            ss << s_t.tm_min;
            ss << s_t.tm_sec;
            ss << "." << cnt++;
            ss << ".log";
            return ss.str();
        }

    private:
        size_t cnt = 0;
        std::string _basename;
        std::ofstream _ofs;
        size_t _cur_size;
        size_t _max_size;
    };

    enum TimeGap
    {
        Sec,
        Min,
        Hour,
        Day
    };

    class TimeRollOutput : public Output
    {
    public:
        using ptr = std::shared_ptr<TimeRollOutput>;
        TimeRollOutput(const std::string &basename, TimeGap gaptype)
            : _basename(basename)
        {
            switch (gaptype)
            {
            case TimeGap::Sec:
                _gap_size = 1;
                break;
            case TimeGap::Min:
                _gap_size = 60;
                break;
            case TimeGap::Hour:
                _gap_size = 3600;
                break;
            case TimeGap::Day:
                _gap_size = 3600 * 24;
                break;
            }
            _cur_gap = _gap_size == 1 ? Util::Date::now() : Util::Date::now() / _gap_size;
            std::string filename = createNewFileName();
            // 如果路径不存在就创建路径, 然后打开文件
            Util::File::create_directory(Util::File::path(filename));
            _ofs.open(filename, ios::app);
            assert(_ofs.is_open());
        }

        void log(const char *data, size_t len)
        {
            // 当前系统时间 / 时间段大小  如果不是当前的时间段, 代表超过了规定的时间, 需要更换文件
            time_t cur = Util::Date::now();
            if ((cur / _gap_size) != _cur_gap)
            {
                // cout << "(cur / _gap_size) != _cur_gap = " << ((cur / _gap_size) != _cur_gap) << endl;
                // cout << "cur time = " << cur << " cur gap = " << _cur_gap << endl;
                // cout << "cur time / cur gap = " << (cur / _gap_size) << endl;
                // cout << "-----------"<<endl;
                _ofs.close();
                std::string filename = createNewFileName();
                _ofs.open(filename, ios::app);
                _cur_gap = _gap_size == 1 ? Util::Date::now() : Util::Date::now() / _gap_size;

                assert(_ofs.is_open());
            }
            _ofs.write(data, len);
            if (!_ofs.good())
            {
                std::cout << "写入滚动文件失败" << std::endl;
            }
        }

    private:
        std::string createNewFileName()
        {
            time_t t = time(nullptr);
            struct tm s_t;
            localtime_r(&t, &s_t);
            std::stringstream ss;
            ss << _basename;
            ss << s_t.tm_year + 1900;
            ss << s_t.tm_mon + 1;
            ss << s_t.tm_mday;
            ss << s_t.tm_hour;
            ss << s_t.tm_min;
            ss << s_t.tm_sec;
            ss << ".log";
            return ss.str();
        }

    private:
        std::string _basename;
        std::ofstream _ofs;
        time_t _cur_gap;
        time_t _gap_size;
    };

    class OutputFactory
    {
    public:
        template <typename OutputType, typename... Args>
        static Output::ptr create(Args &&...args)
        {
            // 使用可变参数返回不同的输出类指针
            return std::make_shared<OutputType>(std::forward<Args>(args)...);
        }
    };
}