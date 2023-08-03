#include <iostream>
using namespace std;

//C++风格的可变参数
//对模板进行特化, 使其容纳最后一个参的情况
template<class T>
void print(const T& val)
{
    cout << val << endl;
}

template<class T, class ...Args>
void print(const T& val, Args... args)
{
    cout << val << " ";
    print(args...);
}

//使用逗号表达式展开
template<class T>
void printArgs(const T& val)
{
    cout << val << " ";
}

template<class ...Args>
void printfunc(Args... args)
{
    int arr[] = {(printArgs(args), 0)...};
    cout << endl;
}

int main()
{
    print("a", 10, '1', 200, "hello sb");
    printfunc("a", 10, '1', 200, "hello sb");
    return 0;
}