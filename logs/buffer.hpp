#pragma once
#include <vector>
#include "util.hpp"

namespace Log
{
// buffer容量增长模式:
// 默认大小为10M, 每次以二倍方式增长, 当达到阈值时, 进行线性增长, 每次增长10M
#define BUFFER_DEFAULT_SIZE (1024 * 1024 * 1)
#define THRESHOLD_SIZE (1024 * 1024 * 8)
#define LINEAR_GROWTH (1024 * 1024 * 1)
    class Buffer
    {
    public:
        Buffer()
            : _reader_pos(0), _writer_pos(0), _buffer(BUFFER_DEFAULT_SIZE)
        {
        }
        // 写入缓冲区
        void push(const char *data, size_t len)
        {
            // 判断是否需要扩容, 如果不需要, 正常插入数据
            //assert(len <= writeableSize());
            expandCapacity(len);
            std::cout << "len = " << len << std::endl;
            std::cout << "write pos = " << _writer_pos << std::endl;
            std::copy(data, data + len, &_buffer[_writer_pos]);
            // 移动写入指针
            std::cout << "copy success" << std::endl;
            moveWriter(len);
        }
        // 移动读指针
        void moveReader(size_t len)
        {
            _reader_pos += len;
        }
        void moveWriter(size_t len)
        {
            _writer_pos += len;
        }

        // 判空
        bool empty()
        {
            return _reader_pos == _writer_pos;
        }
        // 重制
        void reset()
        {
            _reader_pos = _writer_pos = 0;
        }
        void swap(Buffer &buff)
        {
            std::swap(_buffer, buff._buffer);
            std::swap(_reader_pos, buff._reader_pos);
            std::swap(_writer_pos, buff._writer_pos);
        }
        // 返回可写空间大小
        size_t writeableSize()
        {
            return _buffer.size() - _writer_pos;
        }
        // 返回可读数据大小
        size_t readableSize()
        {
            std::cout << "read pos = " << _reader_pos << std::endl;
            std::cout << "write pos = " << _writer_pos << std::endl;
            return _writer_pos - _reader_pos;
        }

        const char *begin()
        {
            return &_buffer[_reader_pos];
        }

    private:
        void expandCapacity(size_t len)
        {
            if (len < writeableSize())
                return; // 空间足够
            size_t new_size = 0;
            if (_buffer.size() < THRESHOLD_SIZE)
            {
                // 小于阈值时, 翻倍增长
                new_size = _buffer.size() * 2 + len;
            }
            else
            {
                // 大于阈值时, 线性增长
                new_size = _buffer.size() + LINEAR_GROWTH + len;
            }
            std::cout << "new size = " << new_size << std::endl;
            _buffer.resize(new_size);
            std::cout << "buffer.size = " << _buffer.size() << std::endl;
        }

    private:
        std::vector<char> _buffer;
        size_t _reader_pos; // 读取位置指针
        size_t _writer_pos; // 写入位置指针
    };
}