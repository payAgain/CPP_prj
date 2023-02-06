//
// Created by YiMing D on 2022/11/19.
//

#ifndef DREAMER_LOG_H
#define DREAMER_LOG_H

#include "basic_log.h"
#include "singleton.h"
#include "file_util.h"

namespace dreamer {

#define DREAMER_SYSTEM_LOGGER() dreamer::LogMgr::getInstance()->get_system_logger()
//#define DREAMER_FILE_ROOT_LOGGER() dreamer::LogMgr::getInstance()->get_file_root_logger()
#define DREAMER_LOGGER(name) dreamer::LogMgr::getInstance()->get_logger(name)
#define DREAMER_LOGGER_MP() dreamer::LogMgr::getInstance()->get_mp()
#define DEFAULT_LOG_CONFIG_PATH "/home/parallels/Desktop/opensource/dreamer/config/config.yaml"

class BasicParser : ParserItem{
public:
    enum Type {
        FILE_NAME = 1000,
        LOG_LEVEL,
        ELAPSE,
        THREAD_ID,
        NEW_LINE,
        LINE_NUM,
        TAB,
        FIBER_ID,
        THREAD_NAME,
        LOGGER_NAME,
        LOGGER_CONTENT
    };
    BasicParser(BasicParser::Type type) :m_type(type) {}
    std::string parser(LogEvent::ptr event) override;

private:
    Type m_type;
};

class DataTimeParser : ParserItem{
public:
    std::string parser(LogEvent::ptr event) override;
    DataTimeParser(std::string pattern = DEFAULT_DATETIME_PATTERN)
            :m_pattern(std::move(pattern)) {}
private:
    std::string m_pattern;
};

class StringParser : ParserItem{
public:
    std::string parser(LogEvent::ptr event) override;
    StringParser(std::string str) : m_str(str) {}

private:
    std::string m_str;
};

class SimpleLogFormatter : public LogFormatter {
public:
    std::string to_string() const override {
        std::stringstream ss;
        ss << "Type: SimpleLogFormatter \n ";
        return ss.str();
    }
    std::string format(LogEvent::ptr) override;
};


class StdLogAppender : public LogAppender{
public:
    void do_append(LogEvent::ptr event) override;
    void append(LogEvent::ptr event) override;
    bool compare(std::string p) override;
    std::string to_string() const override {
        return "StdLogAppender";
    }
};

class FileAppender : public LogAppender {
public:
    FileAppender();
    FileAppender(std::string path);
    void do_append(LogEvent::ptr event) override;
    void append(LogEvent::ptr event) override;
    bool compare(std::string p) override;
    std::string to_string() const override {
        std::stringstream ss;
        if (m_formatter)
            ss << "FileAppender target: " << m_path << "\n Formatter: " << m_formatter->to_string();
        return ss.str();
    }
private:
    std::string m_path;
};

/**
 *
 *  %p 日志级别
 *  %r 累计毫秒数
 *  %c 日志名称
 *  %t 线程id
 *  %n 换行
 *  %d 时间
 *  %f 文件名
 *  %l 行号
 *  %T 制表符
 *  %F 协程id
 *  %N 线程名称
 */

class PatternLogFormatter : public LogFormatter {
public:
    PatternLogFormatter(std::string pattern = DEFAULT_PATTERN) : m_pattern(std::move(pattern)) {}
    std::string format(LogEvent::ptr event) override;

    std::string to_string() const override {
        std::stringstream ss;
        ss << "Type: PatternLogFormatter \n Pattern: " << m_pattern;
        return ss.str();
    }
    int set_pattern(std::string new_pattern);

    int init_items();
private:
    std::vector<ParserItem::ptr> m_items;
    std::string m_pattern;
    static std::map<char, BasicParser::Type> m_basic_pattern;

};

typedef std::map<std::string, Logger::ptr> LOGGER_MAP;
class LogManager {
public:
    LogManager();
    // LogManager(std::string path);
    ~LogManager() = default;
    Logger::ptr get_logger(const std::string& name);
    // get_default_Logger
    Logger::ptr& get_system_logger();
//    Logger::ptr& get_file_root_logger();
    LOGGER_MAP& get_mp() { return m_loggers; }

private:
    void init();
private:
    // LogConfig::ptr m_dconfig;
    Logger::ptr m_system;
//    Logger::ptr m_f_root;
    LOGGER_MAP m_loggers;
};

typedef Singleton<LogManager> LogMgr;

void log_config_cb(LOGGER_MAP old_v, LOGGER_MAP new_v);

}




#endif //DREAMER_LOG_H
