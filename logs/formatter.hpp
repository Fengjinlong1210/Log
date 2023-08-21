#pragma once
#include <cassert>
#include <sstream>
#include <ctime>
#include <vector>
#include "logMsg.hpp"
#include "util.hpp"

namespace Log
{
    class FormatterItem
    {
    public:
        using ptr = std::shared_ptr<FormatterItem>;
        virtual void format(std::ostream &out, const LogMessage &msg) = 0;
    };
    class MsgFormatterItem : public FormatterItem
    {
    public:
        MsgFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << msg._payload;
        }
    };
    class LevelFormatterItem : public FormatterItem
    {
    public:
        LevelFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << Log::LogLevel::levelToStr(msg._lv);
        }
    };
    class NameFormatterItem : public FormatterItem
    {
    public:
        NameFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << msg._name;
        }
    };
    class ThreadFormatterItem : public FormatterItem
    {
    public:
        ThreadFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << msg._tid;
        }
    };
    class FileFormatterItem : public FormatterItem
    {
    public:
        FileFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << msg._file;
        }
    };
    class LineFormatterItem : public FormatterItem
    {
    public:
        LineFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << msg._line;
        }
    };
    class TimeFormatterItem : public FormatterItem
    {
    private:
        std::string _fmt;

    public:
        TimeFormatterItem(const std::string &fmt = "%H:%M:%S") : _fmt(fmt) {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            time_t ctime = msg._ctime;
            struct tm t;
            localtime_r(&ctime, &t); // 将当前时间戳进行结构化
            char buffer[128];
            strftime(buffer, 127, _fmt.c_str(), &t); // 获取当前时间的格式化字符串
            out << buffer;
        }
    };
    class TabFormatterItem : public FormatterItem
    {
    public:
        TabFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << "\t";
        }
    };
    class NewLineFormatterItem : public FormatterItem
    {
    public:
        NewLineFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << "\n";
        }
    };
    class OtherFormatterItem : public FormatterItem
    {
    private:
        std::string _str;

    public:
        OtherFormatterItem(const std::string &str = "") : _str(str) {}
        virtual void format(std::ostream &out, const LogMessage &msg)
        {
            out << _str;
        }
    };

    class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;
        Formatter(const std::string &pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n")
            : _pattern(pattern)
        {
            assert(parsePattern());
        }

        void format(std::ostream &out, const LogMessage &msg)
        {
            // 遍历items, 组合字符串
            for (auto &it : _items)
            {
                it->format(out, msg);
            }
        }

        std::string format(const LogMessage &msg)
        {
            std::stringstream ss;
            format(ss, msg);
            //std::cout << ss.str() << std::endl;
            return ss.str();
        }
        // %d 日期
        // %t 线程ID
        // %c 日志器名称
        // %f 文件名
        // %l 行号
        // %m 消息
        // %T 缩进
        // %n 换行
        // %p 日志级别
        FormatterItem::ptr createItem(const std::string &key, const std::string value)
        {
            if (key == "d")
                return std::make_shared<TimeFormatterItem>(value);
            if (key == "t")
                return std::make_shared<ThreadFormatterItem>(value);
            if (key == "c")
                return std::make_shared<NameFormatterItem>(value);
            if (key == "f")
                return std::make_shared<FileFormatterItem>(value);
            if (key == "l")
                return std::make_shared<LineFormatterItem>(value);
            if (key == "m")
                return std::make_shared<MsgFormatterItem>(value);
            if (key == "T")
                return std::make_shared<TabFormatterItem>(value);
            if (key == "n")
                return std::make_shared<NewLineFormatterItem>(value);
            if (key == "p")
                return std::make_shared<LevelFormatterItem>(value);
            return std::make_shared<OtherFormatterItem>(value);
        }

    private:
        bool parsePattern()
        {
            // 对格式化字符串进行分割
            //  如果没遇到%, 就不需要格式化, 是原始字符串, 直接加入items
            //  如果遇到%, 代表后面是格式化标志, 需要创建一个item, 加入items
            //  如果连续遇到两个%%, 说明需要输出%, 加入items
            //  如果遇到{}, 需要对其中的内容当作一个子项, 加入items

            //用来存储kv对, 最终按顺序放入items中
            std::vector<std::pair<std::string, std::string>> fmt_order;
            std::string key, val;
            int pos = 0;
            while(pos < _pattern.size())
            {
                //不是%, 是原始字符串
                if(_pattern[pos] != '%')
                {
                    val += _pattern[pos++];
                    continue;
                }
                //走到这里说明有两个连续的%
                if(pos + 1 < _pattern.size() && _pattern[pos+1] == '%')
                {
                    val += "%";
                    continue;
                }
                //走到这里说明遇到了一个%, 后面的是格式化字符串, 前面的原始字符串已经处理完
                //需要把原始字符串进行格式化
                if(!val.empty())
                {
                    fmt_order.emplace_back("", val);
                    val.clear();
                }

                pos++;//让pos指向%后面的字符
                if(pos == _pattern.size())
                {
                    std::cout << "%后面没有可用的字符" << std::endl;
                    return false;
                }
                key = _pattern[pos];    //让key等于%后面的字符
                pos++;//此时pos指向%char后面的位置
                if(pos < _pattern.size() && _pattern[pos] == '{')
                {
                    pos++;//如果pos当前为'{', 说明进入了子规则的起始位置
                    while(pos < _pattern.size() && _pattern[pos] != '}')
                    {
                        val += _pattern[pos++];
                    }

                    if(pos == _pattern.size())
                    {
                        //走进这里说明'{'没有找到匹配的'}'
                        std::cout << "子规则匹配出错, '{'没有匹配的'}'" << std::endl;
                        return false;
                    }
                    pos++;
                }
                fmt_order.emplace_back(key, val);
                key.clear();
                val.clear();
            }
            for(auto& fmt : fmt_order)
            {
                _items.push_back(createItem(fmt.first, fmt.second));
            }
            return true;
        }

    private:
        std::string _pattern;
        std::vector<Log::FormatterItem::ptr> _items; // 存放格式化类的父类指针, 可以保存子类对象
    };
}