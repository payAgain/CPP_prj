//
// Created by YiMing D on 2022/11/14.
//

#ifndef DREAMER_BASIC_LOG_H
#define DREAMER_BASIC_LOG_H

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <list>
#include <utility>
#include <vector>
#include <map>

#define DEFAULT_PATTERN "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T"
#define DEFAULT_DATETIME_PATTERN "%Y-%m-%d %H:%M:%S"
#define DEFAULT_LOG_PATH "/Users/yimingd/Documents/log/"

#define D_LOG_FMT(logger, level, fmt...) \
            logger->log(__FILE__, __LINE__, level, fmt)

#define D_LOG_INFO(logger, fmt...) \
            D_LOG_FMT(logger, dreamer::LogLevel::INFO, fmt)

#define D_LOG_DEBUG(logger, fmt...) \
            D_LOG_FMT(logger, dreamer::LogLevel::DEBUG, fmt)

#define D_LOG_WARN(logger, fmt...) \
            D_LOG_FMT(logger, dreamer::LogLevel::WARN, fmt)

#define D_LOG_ERROR(logger, fmt...) \
            D_LOG_FMT(logger, dreamer::LogLevel::ERROR, fmt)

#define D_LOG_FATAL(logger, fmt...) \
            D_LOG_FMT(logger, dreamer::LogLevel::FATAL, fmt)

#define D_LOG_STREAM(logger, level) \
            if (logger->get_level() <= level) \
                dreamer::LogEventWrap(logger, __FILE__, __LINE__, level).get_ss()

#define D_SLOG_DEBUG(logger) \
            D_LOG_STREAM(logger, dreamer::LogLevel::Level::DEBUG)

#define D_SLOG_INFO(logger) \
            D_LOG_STREAM(logger, dreamer::LogLevel::Level::INFO)

#define D_SLOG_WARN(logger) \
            D_LOG_STREAM(logger, dreamer::LogLevel::Level::WARN)

#define D_SLOG_ERROR(logger) \
            D_LOG_STREAM(logger, dreamer::LogLevel::Level::ERROR)

#define D_SLOG_FETAL(logger) \
            D_LOG_STREAM(logger, dreamer::LogLevel::Level::FETAL)


namespace dreamer {

class LogLevel {
public:
    enum Level{
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5,
        UNKNOWN = 0
    };
    static std::string to_string(Level level);
};

/**
 * @brief Log时间
 * @todo 属性get方法
 */
class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent(const char* file, int32_t line, uint32_t elapse
            , uint32_t thread_id, uint32_t fiber_id, uint64_t time
            , std::string thread_name, LogLevel::Level level
            , std::string logger_name);

    const char* get_file_name() { return m_file; }
    int32_t get_line() const { return m_line; }
    uint32_t get_thread_id() const { return m_thread_id; }
    uint32_t get_elapse() const { return m_elapse; }
    uint32_t get_fiber_id() const { return m_fiber_id; }
    uint64_t get_time_stamp() const { return m_time_stamp; }
    std::string get_thread_name() { return m_thread_name; }
    LogLevel::Level get_level() { return m_level; }
    std::stringstream& get_ss() { return m_stringStream; }
    std::string get_logger() { return m_logger_name; }


private:

    const char* m_file = nullptr; // 文件名
    int32_t m_line = 0; // 行号s
    uint32_t m_elapse = 0; // 程序启动开始到现在的毫秒数
    uint32_t m_thread_id = 0; // 线程ID
    uint32_t m_fiber_id = 0; // 协程ID
    uint64_t m_time_stamp = 0; // 时间戳
    std::string m_thread_name;  // 线程名称
    std::stringstream m_stringStream; // 日志内容流
    LogLevel::Level m_level; // 日志等级
    std::string m_logger_name; // 日志器名称
};

//Formatter 由Appender调用
class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;

    virtual ~LogFormatter() = default;
    virtual std::string format(LogEvent::ptr) = 0;
    virtual std::string to_string() const = 0;
//    virtual std::string to_yml();
//    virtual bool set_config(std::string);
private:
};

class ParserItem {
public:
    using ptr = std::shared_ptr<ParserItem>;
    virtual std::string parser(LogEvent::ptr event) = 0;
    virtual ~ParserItem() = default;
};

class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;

    LogAppender() = default;
    virtual void do_append(LogEvent::ptr event) = 0;
    virtual void append(LogEvent::ptr event) = 0;
    virtual bool compare(std::string p) = 0;
    virtual std::string to_string() const = 0;
    virtual ~LogAppender() = default;
    void set_formatter(LogFormatter::ptr formatter) { m_formatter = std::move(formatter); }
protected:
    LogFormatter::ptr m_formatter;
};

class Logger {
public:
    using ptr = std::shared_ptr<Logger>;

    Logger();
    Logger(LogLevel::Level logger_level, std::string logger_name);

    void log(const char* file, int32_t line, LogLevel::Level log_level, const char* fmt...);
    void log(const LogEvent::ptr& event);
//    为了利用__FILE__和__LINE__宏，因此移除这些函数
//    void debug(const char* fmt, ...);
//    void info(const char* fmt, ...);
//    void warn(const char* fmt, ...);
//    void error(const char* fmt, ...);
//    void fatal(const char* fmt, ...);

//    std::stringstream& operator<<(const std::string& message);

    std::string to_string();
    // getter
    LogLevel::Level get_level() { return m_logger_level; }
    bool is_autoNewLine() { return m_default_newLine; }
    std::string get_name() const { return m_logger_name; }

    // setter
    void set_level(LogLevel::Level level) { m_logger_level = level; }
    void set_autoNewLine(bool t) { m_default_newLine = t; }
    void add_appender(const LogAppender::ptr& appender);
    void set_appender(const std::list<LogAppender::ptr>& appender);
    void del_appender(const LogAppender::ptr& appender);
    void clear_appender() { m_appender.clear(); }

private:
    std::string m_logger_name;
    bool m_default_newLine = true;
    LogLevel::Level m_logger_level; // Logger级别 低于该级别的不输出
    std::list<LogAppender::ptr> m_appender;
//    std::stringstream m_stream; // 流式输出
};


class LogEventWrap {
public:
    LogEventWrap(Logger::ptr &logger, const char* file, int32_t line, LogLevel::Level log_level);
    ~LogEventWrap();

    std::stringstream& get_ss() {
        return m_event->get_ss();
    }

    std::stringstream& operator<<(const std::string &str) {
        auto &t =  m_event->get_ss();
        t << str;
        return t;
    }
private:
    Logger::ptr m_logger;
    LogEvent::ptr m_event;
};

}



#endif //DREAMER_BASIC_LOG_H
