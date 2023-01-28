//
// Created by YiMing D on 2022/11/14.
//

#include <utility>
#include <cstdarg>
#include "d_thread.h"
#include "basic_log.h"
#include "fiber.h"

namespace dreamer {

// 实现LogEvent
LogEvent::LogEvent(const char* file, int32_t line, uint32_t elapse
            , uint32_t thread_id, uint32_t fiber_id, uint64_t time_stamp
            , std::string thread_name, LogLevel::Level level, std::string logger_name)
            :m_file(file)
            ,m_line(line)
            ,m_elapse(elapse)
            ,m_thread_id(thread_id)
            ,m_fiber_id(fiber_id)
            ,m_time_stamp(time_stamp)
            ,m_thread_name(std::move(thread_name))
            ,m_level(level)
            ,m_logger_name(std::move(logger_name)){}

std::string LogLevel::to_string(Level level) {
    switch (level)
    {
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::WARN:
        return "WARN";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

// 实现Logger
Logger::Logger() {
    m_logger_name = "root";
    m_logger_level = LogLevel::Level::DEBUG;
}

Logger::Logger(LogLevel::Level logger_level, std::string logger_name)
            : m_logger_level(logger_level)
            , m_logger_name(std::move(logger_name)) {}


void Logger::log(const char* file, int32_t line,
                 LogLevel::Level log_level, const char* fmt...) {
    MutexLock mutexLock(lock);
    if (log_level >= m_logger_level) {
        char* buf = nullptr;
        LogEvent::ptr new_event(new LogEvent(file, line
                    , clock(), GetThreadId(), Fiber::GetFiberId(), time(nullptr), Thread::GetThreadName(), log_level, m_logger_name));
        std::stringstream& ss = new_event->get_ss();
        va_list al;
        va_start(al, fmt);
        int len = vasprintf(&buf, fmt, al);
        if (len != -1) {
            ss << buf;
//            if (is_autoNewLine()) {
//                ss << std::endl;
//            }
            free(buf);
        } else {
            perror("日志格式转换失败");
        }
        va_end(al);
        for(auto& appender : m_appender) {
            appender->do_append(new_event);
        }
    }
}

void Logger::log(const LogEvent::ptr& event) {
    MutexLock mutexLock(lock);
    for(auto& appender : m_appender) {
        appender->do_append(event);
    }
}

void Logger::add_appender(const LogAppender::ptr& appender) {
    m_appender.push_back(appender);
}
std::string Logger::to_string() {
    std::stringstream ss;
    ss << "name: " << get_name() << "\n level: " << get_level();
    for(auto &it : m_appender) {
        ss << "\n appender: \n" << it->to_string();
    }
    return ss.str();
}

void Logger::set_appender(const std::list<LogAppender::ptr> &appender) {
    m_appender = appender;
}

void Logger::del_appender(const LogAppender::ptr& appender) {
    for (auto& app : m_appender) {
        if (app == appender) {
            m_appender.remove(app);
        }
    }
}

void Logger::set_logger(Logger::ptr new_logger) {
    m_logger_level = new_logger->get_level();
    m_appender = new_logger->m_appender;
}


// 实现LogEventWrap
LogEventWrap::LogEventWrap(Logger::ptr &logger, const char* file, int32_t line, LogLevel::Level log_level)
                        : m_logger(logger)
                        , m_event(new LogEvent(file, line , clock(), GetThreadId(), Fiber::GetFiberId()
                             , time(nullptr), Thread::GetThreadName(), log_level, m_logger->get_name())) {}



LogEventWrap::~LogEventWrap() {
//    if (m_logger->is_autoNewLine()) {
//        m_event->get_ss() << std::endl;
//    }
    m_logger->log(m_event);
}


}

