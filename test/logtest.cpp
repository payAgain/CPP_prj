#include "../include/log.h"
#include "pthread.h"
#include "chrono"
#include "my_thread.h"
#include "iostream"
#include "thread"

void f() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    dreamer::Logger log = dreamer::Logger(dreamer::LogLevel::INFO, "root");
    dreamer::LogAppender::ptr appender((dreamer::LogAppender *)new dreamer::StdLogAppender());
    dreamer::LogFormatter::ptr formatter((dreamer::LogFormatter *)
                                                 new dreamer::PatternLogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%r%T%t%T%N%T%F%T[%p]%TLoger:[%c]%T%f:%l message:%T"));
    appender->set_formatter(formatter);
    log.add_appender(appender);
    const char *p = "牛马";
    D_LOG_INFO(log, "This is a message");
}

int main() {
    dreamer::Logger log = dreamer::Logger(dreamer::LogLevel::INFO, "root");
    dreamer::LogAppender::ptr appender((dreamer::LogAppender *)new dreamer::StdLogAppender());
    dreamer::LogFormatter::ptr formatter((dreamer::LogFormatter *)
                            new dreamer::PatternLogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%TLoger:[%c]%T%f:%l message:%T"));
    appender->set_formatter(formatter);
    log.add_appender(appender);
    const char *p = "牛马";
//    D_LOG_INFO(log, "This is a message");
//    D_LOG_INFO(log, "李志成是%s", p);
//    D_LOG_ERROR(log, "%s,周老板,%s", p, p);
    dreamer::ThreadGuard t(f);
    t.set_thread_name("Thread 1");
    t.join();
}