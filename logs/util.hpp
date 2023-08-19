#pragma once
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace Log
{
    namespace Util
    {
        class Date
        {
        public:
            static size_t now()
            {
                return (size_t)time(nullptr);
            }
        };

        class File
        {
        public:
            // 判断路径或文件是否存在
            static bool exists(const std::string &name)
            {
                struct stat st;
                int n = stat(name.c_str(), &st);
                return n == 0;
            }

            // 获取一个路径目录 ./abc/test.txt -> ./abc
            static std::string path(const std::string &path)
            {
                if (path.empty())
                    return ".";
                auto pos = path.find_last_of("/\\");
                // '/' 表示Linux下的路分隔符 '\\'表示Windows下的路径分隔符
                if (pos == std::string::npos)
                    return "."; // 没有路径分隔符
                return path.substr(0, pos + 1);
            }

            static void create_directory(const std::string &path)
            {
                if (path.empty())
                    return;
                //std::cout << "not empty" << std::endl;
                if (exists(path))
                    return; // 路径已经存在
                //std::cout << "exist" << std::endl;
                size_t pos = 0, offset = 0;
                while (offset < path.size())
                {
                    pos = path.find_first_of("/\\", offset);
                    std::cout << "pos = " << pos << std::endl;
                    if (pos == std::string::npos)
                    {
                        mkdir(path.c_str(), 0777);
                        return;
                    }
                    std::string parent_dir = path.substr(0, pos + 1);
                    if (exists(parent_dir))
                    {
                        offset = pos + 1;
                        continue;
                    }
                    mkdir(parent_dir.c_str(), 0777);
                    offset = pos + 1;
                }
            }
        };
    }
}