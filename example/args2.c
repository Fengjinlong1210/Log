//C风格不定参函数
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
//使用va_list 创建一个指向可变参数的指针
void printNum(int n, ...)
{
    va_list p;
    va_start(p, n); //让指针p指向n参数后面的参数
    for(int i = 0; i < n; ++i)
    {
        int num = va_arg(p, int);   
        //va_arg表示获取参数列表中的参数,以type类型返回,并将指针移到下一个参数开始
        printf("%d ", num);
    }
    printf("\n");
    va_end(p);
}

void myprintf(const char* fmt, ...)
{
    char* res;
    va_list p;
    va_start(p, fmt);
    vasprintf(&res, fmt, p);
    printf(res);
    va_end(p);
    free(res);
}

int main()
{
    printNum(5, 1, 2, 3, 4, 5);
    myprintf("%d, hello \n", 10);
    return 0;
}