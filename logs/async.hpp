#pragma once
#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "buffer.hpp"

namespace Log
{
    // 该类实现两个缓冲区的交换, 当消费缓冲区为空时,
    enum AsyncType
    {
        ASYNC_SAFE,
        ASYNC_UNSAFE,
    };
    using functor = std::function<void(Buffer &)>;
    class AsyncLooper
    {
    public:
        using ptr = std::shared_ptr<AsyncLooper>;
        AsyncLooper(const functor &cb, AsyncType async_type = AsyncType::ASYNC_SAFE)
            : _stop(false),
              _thread(std::thread(&AsyncLooper::threadEntry, this)),
              _callback(cb),
              _async_type(async_type)
        {
            //std::cout << "AsyncLooper construction"<< std::endl;
        }
        ~AsyncLooper()
        {
            //std::cout << "AsyncLooper destruction"<< std::endl;
            stop();
        }
        void stop()
        {
            _stop = true;                // 将标记为置为true, 表示退出
            _cond_consumer.notify_all(); // 通知所有的消费者, 让消费者线程退出
            _thread.join();
        }

        void push(const char *data, size_t len)
        {
            // std::cout << "push :: data = " << data << std::endl;
            // std::cout << "push :: len = " << len << std::endl;

            // 设置临界资源的访问
            std::unique_lock<std::mutex> lock(_mutex);
            // 判断缓冲区是否满足条件(不扩容模式下才需要判断)
            if (_async_type == AsyncType::ASYNC_SAFE)
                _cond_producer.wait(lock, [&]
                                    { return _buff_producer.writeableSize() >= len; });
            // 向生产缓冲区压入数据
            _buff_producer.push(data, len);
            // 唤醒消费者线程对缓冲区数据进行处理
            _cond_consumer.notify_one();
        }

    private:
        void threadEntry()
        {
            while (1)
            {
                // 设置一段临界区, 只对缓冲区的交换进行上锁, 不对数据处理上锁
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    // 当退出状态为真且生产缓冲区为空时退出
                    if (_stop && _buff_producer.empty())
                    {
                        //std::cout << _buff_consumer.getReadableData();
                        //std::cout << _buff_producer.getReadableData();
                        //std::cout << "async looper exit" << std::endl;
                        break;
                    }
                    // 如果退出状态为真, 或者生产缓冲区不为空时, 唤醒消费者线程, 否则继续休眠
                    _cond_consumer.wait(lock, [&]
                                        { return !_stop || !_buff_producer.empty(); });
                    // 生产缓冲区有数据, 交换两个缓冲区
                    //_buff_consumer.swap(_buff_producer);
                    _buff_producer.swap(_buff_consumer);
                    // 唤醒生产者
                }
                if (_async_type == AsyncType::ASYNC_SAFE)
                    _cond_producer.notify_all();
                // 被唤醒后, 处理消费缓冲区中的数据
                _callback(_buff_consumer);
                // std::cout << "readable size = " << _buff_consumer.readableSize() << std::endl;
                //  重制缓冲区
                _buff_consumer.reset();
            }
            //std::cout << "while exit" << std::endl;
        }

    private:
        std::atomic<bool> _stop;                // 判断是否退出
        std::mutex _mutex;                      // 互斥锁
        std::condition_variable _cond_consumer; // 消费者的条件变量
        std::condition_variable _cond_producer; // 生产者的条件变量
        Buffer _buff_consumer;                  // 消费者的缓冲区
        Buffer _buff_producer;                  // 生产者的缓冲区
        std::thread _thread;                    // 创建线程, 用来执行缓冲区的交换
        AsyncType _async_type;
        functor _callback; // 回调函数
    };
}