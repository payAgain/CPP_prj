#include "../include/log.h"
#include "pthread.h"
#include "my_thread.h"
#include "iostream"
#include "thread"

int main() {
    dreamer::Logger log = dreamer::Logger(dreamer::LogLevel::INFO, "root");
    dreamer::LogAppender::ptr appender((dreamer::LogAppender *)new dreamer::StdLogAppender());
    dreamer::LogFormatter::ptr formatter((dreamer::LogFormatter *)
                            new dreamer::PatternLogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%TLoger:[%c]%T%f:%l message:%T"));
    appender->set_formatter(formatter);
    log.add_appender(appender);
    const char *p = "牛马";
    D_LOG_INFO(log, "This is a message");
    D_LOG_INFO(log, "李志成是%s", p);
    D_LOG_ERROR(log, "%s,周老板,%s", p, p);
    std::stringstream ss;
    pthread_setname_np("Name");
    char buf[100];
    pthread_t t = pthread_self();
    pthread_getname_np(t, buf, sizeof buf);
    std::cout << buf;
//    auto t = std::this_thread::get_id();
//    std::cout << t;
//    std::cout << std::this_thread::get_id();
}