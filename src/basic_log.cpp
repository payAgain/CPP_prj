//
// Created by YiMing D on 2022/11/14.
//

#include <utility>

#include "../include/basic_log.h"

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
Logger::Logger(LogLevel::Level logger_level, std::string logger_name)
            : m_logger_level(logger_level)
            , m_logger_name(std::move(logger_name)) {}


void Logger::log(const char* file, int32_t line,
                 LogLevel::Level logger_level, const char* fmt...) {
    if (logger_level >= m_logger_level) {
        char* buf = nullptr;
        LogEvent::ptr new_event(new LogEvent(file, line
                    , 0, 0, 0, 0, "thread_name", logger_level, m_logger_name));
        std::stringstream& ss = new_event->get_ss();
        va_list al;
        va_start(al, fmt);
        int len = vasprintf(&buf, fmt, al);
        if (len != -1) {
            ss << buf;
            free(buf);
        } else {
            perror("日志格式转换失败");
        }
        va_end(al);
        for(auto& appender : m_logger_appenders) {
            appender->do_append(new_event);
        }
    }
}

void Logger::add_appender(const LogAppender::ptr& appender) {
    m_logger_appenders.push_back(appender);
}

void Logger::del_appender(const LogAppender::ptr& appender) {
    for (auto& app : m_logger_appenders) {
        if (app == appender) {
            m_logger_appenders.remove(app);
        }
    }
}





}

