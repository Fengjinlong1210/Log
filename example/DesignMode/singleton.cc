#include <iostream>
using namespace std;
/*
//饿汉模式
class Singleton
{
public:
    static Singleton* get_instance()
    {
        return &_instance;
    }

    void get_data()
    {
        cout << _data << endl;
    }
private:
    Singleton() :_data(100){}
    ~Singleton(){}
    Singleton(const Singleton&) = delete;
private:
    static Singleton _instance;
    int _data;
};
Singleton Singleton::_instance;

int main()
{
    auto instance = Singleton::get_instance();
    instance->get_data();

    return 0;
}*/

//懒汉模式: 第一次使用的时候才实例化
class Singleton
{
public:
    static Singleton* get_instance()
    {
        //C++11之后, 多个线程同时试图同时初始化一个静态变量, 该变量只会被初始化一次
        //也就是保证静态变量初始化时的线程安全
        static Singleton _instance;
        return &_instance;
    }

    void get_data()
    {
        cout << _data << endl;
    }
private:
    Singleton() :_data(100){}
    ~Singleton(){}
    Singleton(const Singleton&) = delete;
private:
    int _data;
};

int main()
{
    auto instance = Singleton::get_instance();
    auto instance2 = Singleton::get_instance();

    instance->get_data();
    instance2->get_data();
}