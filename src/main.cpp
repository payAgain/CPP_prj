//
// Created by YiMing D on 2022/11/14.
//
#include "log.h"
#include "thread"
#include "d_thread.h"

class X
{
public:
    void do_lengthy_work(int);
};
void f(int i, std::string const& s) {
    std::cout << s;
}
int main()
{

    X my_x;
    int num(0);
    std::thread t(f, 3, "hello");
//    std::thread t(&X::do_lengthy_work, &my_x, num);
}