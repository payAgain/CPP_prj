//
// Created by YiMing D on 2022/11/19.
//

#ifndef DREAMER_LOG_H
#define DREAMER_LOG_H

#include "basic_log.h"
#include "singleton.h"
#include "file_util.h"

namespace dreamer {


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

class SimpleLogFormatter : LogFormatter {
public:
    std::string format(LogEvent::ptr) override;
};


class StdLogAppender : LogAppender{
public:
    void do_append(LogEvent::ptr event) override;
    void append(LogEvent::ptr event) override;
};

class FileAppender : LogAppender {
public:
    FileAppender();
    void do_append(LogEvent::ptr event) override;
    void append(LogEvent::ptr event) override;
private:
    std::string m_path;
    std::string m_fileName;
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

class PatternLogFormatter : LogFormatter {
public:
    PatternLogFormatter(std::string pattern = DEFAULT_PATTERN) : m_pattern(std::move(pattern)) {}
    std::string format(LogEvent::ptr event) override;

    int set_pattern(std::string new_pattern);

    int init_items();
private:
    std::vector<ParserItem::ptr> m_items;
    std::string m_pattern;
    static std::map<char, BasicParser::Type> m_basic_pattern;

};


class LogManager {
public:
    LogManager();
    // LogManager(std::string path);
    ~LogManager() = default;
    Logger::ptr get_Logger(const std::string& name);
    // get_default_Logger
    Logger::ptr get_StdLogger();
    Logger::ptr get_FileLogger();
private:
    void init();
private:
    // LogConfig::ptr m_dconfig;
    Logger::ptr m_gs_root;
    Logger::ptr m_gf_root;
    std::map<std::string, Logger::ptr> m_loggers;
};

typedef Singleton<LogManager> LogMgr;

static Logger::ptr gs_logger = LogMgr::getInstance()->get_StdLogger();
static Logger::ptr gf_logger = LogMgr::getInstance()->get_FileLogger();



}


#endif //DREAMER_LOG_H
