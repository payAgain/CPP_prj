//
// Created by YiMing D on 2022/11/14.
//

#include <utility>
#include <stdarg.h>
#include "my_thread.h"
#include "basic_log.h"
#include "log.h"

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
                    , clock(), get_thread_id(), 0, time(nullptr), get_thread_name(), logger_level, m_logger_name));
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




// 实现Config

LogConfig::LogConfig() {
    m_name = "root";
    m_appenders.emplace_back((AppenderConfig *)new StdAppenderConfig());
    // m_formatter = "PatternLogFormatter";
    // m_pattern = DEFAULT_PATTERN;
    m_level = LogLevel::DEBUG;
    m_changed = true;
}

Logger::ptr LogConfig::create_logger() {
    if (!m_changed) return m_logger;
    m_logger = std::make_shared<Logger>(new Logger(m_level, m_name));
    for (auto &t : m_appenders) {
        m_logger->add_appender(t->get_appender());
    }
    return m_logger;
}

/**
 * @todo 
*/
bool LogConfig::set_config(std::string YMAL) {
    return 0;
}
/**
 * @todo 
*/
LogConfig::LogConfig(std::string YAML) {

}
/**
 * @todo 
*/
std::string LogConfig::to_YMAL() {
    return "";
}


// 实现LogFactory

LogFactory::LogFactory() {
    m_dconfig = std::make_shared<LogConfig>(new LogConfig());
    
}





}

