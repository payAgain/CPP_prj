#include "log.h"
#include "pthread.h"
#include "chrono"
#include "d_thread.h"
#include "iostream"
#include "thread"


void f() {

//    1. 正常的创建方式
    //    dreamer::set_thread_name("MiaoMiao");
//    std::this_thread::sleep_for(std::chrono::seconds(2));
//    dreamer::Logger log = dreamer::Logger(dreamer::LogLevel::INFO, "root");
//    dreamer::LogAppender::ptr appender((dreamer::LogAppender *)new dreamer::StdLogAppender());
//    dreamer::LogFormatter::ptr formatter((dreamer::LogFormatter *)
//                                                 new dreamer::PatternLogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%r%T%t%T%N%T%F%T[%p]%TLoger:[%c]%T%f:%l message:%T"));
//    appender->set_formatter(formatter);
//    log.add_appender(appender);

//    2. 使用LogManager创建实例
//    dreamer::LogManager logManager;

//    3. 使用单例模式构造LogManager
    auto t = dreamer::LogMgr::getInstance();
    auto log = t->get_system_logger();
    D_LOG_INFO(log, "An Error Occurred");
    D_LOG_ERROR(log, "An Error Occurred");
}

int main() {
//    dreamer::Logger log = dreamer::Logger(dreamer::LogLevel::INFO, "root");
//    dreamer::LogAppender::ptr appender((dreamer::LogAppender *)new dreamer::StdLogAppender());
//    dreamer::LogFormatter::ptr formatter((dreamer::LogFormatter *)
//                            new dreamer::PatternLogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%TLoger:[%c]%T%f:%l message:%T"));
//    appender->set_formatter(formatter);
//    log.add_appender(appender);


//    D_LOG_INFO(log, "This is a message");
//    D_LOG_INFO(log, "李志成是%s", p);
//    D_LOG_ERROR(log, "%s,周老板,%s", p, p);

    std::cout << "This is Info" << std::endl;
//    D_LOG_INFO(dreamer::DREAMER_SYSTEM_LOGGER(), "An Error Occurred");
//    D_LOG_ERROR(dreamer::DREAMER_SYSTEM_LOGGER(), "An Error Occurred");
//
//    D_SLOG_DEBUG(dreamer::DREAMER_SYSTEM_LOGGER()) << "This is " << std::endl;

}