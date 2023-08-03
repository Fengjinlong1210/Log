//C语言不定参数宏
#include <stdio.h>
#include <stdarg.h>

#define LOG(fmt, ...) printf("[%s:%d]" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
//在__VA_ARGS__前加上##可以在没有参数的时候去掉前面的','
//##的作用就是连接前后两部分, 充当粘合剂
int main()
{
    LOG("hello");
    return 0;
}