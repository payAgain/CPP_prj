//
// Created by YiMing D on 2022/11/19.
//

#include "log.h"
#include "config.h"

#include <memory>

namespace dreamer{


 // 实现PatternParser
 std::string BasicParser::parser(LogEvent::ptr event) {
     switch (m_type) {
         case FILE_NAME:
             return {event->get_file_name()};
         case LOG_LEVEL:
             return LogLevel::to_string(event->get_level());
         case ELAPSE:
             return std::to_string(event->get_elapse() / (double)CLOCKS_PER_SEC) + "sec";
         case THREAD_ID:
             return std::to_string(event->get_thread_id());
         case NEW_LINE:
             return "\n";
         case LINE_NUM:
             return std::to_string(event->get_line());
         case TAB:
             return "   ";
         case FIBER_ID:
             return std::to_string(event->get_fiber_id());
         case THREAD_NAME:
             return event->get_thread_name();
         case LOGGER_NAME:
             return event->get_logger();
         case LOGGER_CONTENT:
             return event->get_ss().str();
         default:
             return "Unknown";
     }
 }

 std::string DataTimeParser::parser(LogEvent::ptr event) {
     time_t time1 = event->get_time_stamp();
//     time(&time1);
     tm* t = localtime(&time1);
     char s[40];
     strftime(s, sizeof(s), m_pattern.c_str(), t);
     return {s};
 }

std::string StringParser::parser(LogEvent::ptr event) {
     return m_str;
 }

 // 实现SimpleFormatter
 std::string SimpleLogFormatter::format(LogEvent::ptr event) {
     MutexLock mt(lock);
     std::stringstream ss;
     ss << '[' << LogLevel::to_string(event->get_level())
        << "]---";
     return ss.str();
 }


 // 实现PatternLogFormatter
 std::string PatternLogFormatter::format(LogEvent::ptr event) {
     MutexLock mt(lock);
     if (m_items.empty()) {
         if (init_items() == -1) {
             perror("Unexpect error occurred while parse Pattern");
             return "Unknown Pattern";
         }
     }
     std::stringstream ss;
     for(auto &t : m_items) {
         ss << t->parser(event);
     }
     return ss.str();
 }

 int PatternLogFormatter::init_items() {
     std::vector<ParserItem::ptr> new_items;
     size_t n = m_pattern.size();
     std::stringstream ss;
     for(size_t i = 0;i < n; i++) {
         if (m_pattern[i] == '%') {
             if (i + 1 < n) {
                 char tmp = m_pattern[i + 1];
                 if (m_basic_pattern.contains(tmp)) {
                     if (!ss.str().empty()) {
                         new_items.emplace_back((ParserItem *)
                                                        new StringParser(ss.str()));
                         ss.str("");
                     }
                     new_items.emplace_back((ParserItem *)
                                                    new BasicParser(PatternLogFormatter::m_basic_pattern[tmp]));
                     i++;
                 } else if (tmp == 'd') {
                     if (!ss.str().empty()) {
                         new_items.emplace_back((ParserItem *)
                                                        new StringParser(ss.str()));
                         ss.str("");
                     }
                     // 解析日期格式
                     if (i + 2 < n && m_pattern[i + 2] == '{') {
                         int j = i + 3;
                         while(j < n && m_pattern[j] != '}') j++;
                         if (m_pattern[j] == '}') {
                             std::string date_pattern = m_pattern.substr(i + 3, j - i - 3);
                             new_items.emplace_back((ParserItem *) new DataTimeParser(date_pattern));
                         } else {
                             perror("日志格式解析出错!");
                             return -1;
                         }
                         i = j;
                     } else {
                         new_items.emplace_back((ParserItem *) new DataTimeParser());
                     }
                 } else {
                     ss << m_pattern[i];
                 }
             }
             else {
                 ss << m_pattern[i];
             }
         }
         else {
             ss << m_pattern[i];
         }
     }
     if (!ss.str().empty()) {
         new_items.emplace_back((ParserItem *)
                                        new StringParser(ss.str()));
         ss.str("");
     }
     m_items = new_items;
     return 0;
 }

 int PatternLogFormatter::set_pattern(std::string new_pattern) {
     std::string old_pattern = m_pattern;
     m_pattern = std::move(new_pattern);
     if (init_items() == -1) {
         m_pattern = old_pattern;
         perror("设置新匹配模式失败");
         return -1;
     }
     return 0;
 }


 std::map<char, BasicParser::Type> PatternLogFormatter::m_basic_pattern = {
         {'p', BasicParser::LOG_LEVEL},
         {'r', BasicParser::ELAPSE},
         {'c', BasicParser::LOGGER_NAME},
         {'t', BasicParser::THREAD_ID},
         {'n', BasicParser::NEW_LINE},
         {'f', BasicParser::FILE_NAME},
         {'l', BasicParser::LINE_NUM},
         {'T', BasicParser::TAB},
         {'F', BasicParser::FIBER_ID},
         {'N', BasicParser::THREAD_NAME},
         {'S', BasicParser::LOGGER_CONTENT}
 };

 // 实现StdLogAppender
 void StdLogAppender::do_append(LogEvent::ptr event) {
     // @todo Filter

     append(event);
 }

 void StdLogAppender::append(LogEvent::ptr event) {
     MutexLock mt(lock);
     if (m_formatter != nullptr) {
         std::cout << m_formatter->format(event);
     }
//     std::cout << event->get_ss().str();
 }

 bool StdLogAppender::compare(std::string p) {
     return "StdLogAppender" == p;
 }

 // 实现FileAppender
 FileAppender::FileAppender() {
    m_path = DEFAULT_LOG_PATH + get_Today();
 }

 FileAppender::FileAppender(std::string path) {
     m_path = path;
 }

 bool FileAppender::compare(std::string p) {
     return p == m_path;
 }

 void FileAppender::do_append(LogEvent::ptr event) {
     // @todo Filter

     append(event);
 }

 void FileAppender::append(LogEvent::ptr event) {
     MutexLock mt(lock);
    FileOperation op;
    if (op.open(m_path)) {
        D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "文件: " << m_path << "打开失败";
        return;
    }
     if (m_formatter != nullptr) {
         op.write(m_formatter->format(event));
     }
//    op.write(event->get_ss().str());
 }

LogManager::LogManager() {
    init();
}

Logger::ptr& LogManager::get_system_logger() {
    return m_loggers["system"];
}
//Logger::ptr& LogManager::get_file_root_logger() {
//    return m_f_root;
//}
void LogManager::init() {
    m_system = std::make_shared<Logger>(LogLevel::DEBUG, "system");
//    m_f_root = std::make_shared<Logger>(LogLevel::DEBUG, "stdRoot");
    LogAppender::ptr apd_s{(LogAppender *)new StdLogAppender()};
//    LogAppender::ptr apd_f{(LogAppender *)new FileAppender()};
    LogFormatter::ptr lft_s{(LogFormatter *) new PatternLogFormatter()};
//    LogFormatter::ptr lft_f{(LogFormatter *) new PatternLogFormatter()};
    apd_s->set_formatter(lft_s);
//    apd_f->set_formatter(lft_f);
    m_system->add_appender(apd_s);
//    m_f_root->add_appender(apd_f);
    m_loggers["system"] = m_system;
//    m_loggers["fileRoot"] = m_f_root;
 }
Logger::ptr LogManager::get_logger(const std::string& name) {
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }
    return m_system;
}

void log_config_cb(LOGGER_MAP old_v, LOGGER_MAP new_v) {
    auto& mp = DREAMER_LOGGER_MP();
    for(auto &it : new_v) {
        auto t = mp.find(it.first);
        if (t == mp.end()) {
            mp[it.first] = it.second;
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "Logger配置更改 新增: " << it.first;
        } else {
            t->second = it.second;
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "Logger配置修改 变更的logger: " << it.first;
        }
    }
}

class LogConfig {
public:
     LogConfig() {
         dreamer::ConfigMgr::getInstance()->look_up("loggers", "loggers", dreamer::LOGGER_MAP());
         DREAMER_ROOT_CONFIG()->loadConfig(dreamer::YMLParser(), "/Users/yimingd/Desktop/opensource/dreamer/config/config.yaml");
     }
};

static LogConfig logConfig;


}