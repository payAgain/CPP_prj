//
// Created by YiMing D on 2022/12/27.
//

#ifndef DREAMER_CONFIG_H
#define DREAMER_CONFIG_H

#include "log.h"
#include "unordered_map"
#include "utils.h"
#include "boost/lexical_cast.hpp"
#include "file_util.h"
#include "yaml-cpp/yaml.h"

namespace dreamer {

class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;

    ConfigVarBase() = default;
    ConfigVarBase(std::string& name, std::string& desc)
                    : m_name(name), m_desc(desc) {}
    virtual ~ConfigVarBase() = default;

    virtual bool from_string(std::string config) = 0;
    virtual std::string to_string() const = 0;
    virtual std::string get_type() const = 0;

//            if (nodes.IsSequence()) {
    std::string get_name() const { return m_name; }
    std::string get_desc() const { return m_desc; }
private:
    std::string m_name;
    std::string m_desc;
};

template<class V>
class StringToV {
public:
    V operator()(const std::string& f) {
        return boost::lexical_cast<V>(f);
    }
};

template<class V>
class VToString {
public:
    std::string operator()(V v) {
        return boost::lexical_cast<std::string>(v);
    }
};

template<class V>
class StringToV<std::list<V>> {
public:
    std::list<V> operator()(const std::string& f) {
        YAML::Node nodes = YAML::Load(f);
        std::list<V> res;
        if (nodes.IsSequence()) {
            for (auto it : nodes) {
                std::stringstream ss;
                ss << it;
                res.push_back(StringToV<V>()(ss.str()));
            }
        } else {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Config is not a Sequence The type is " << nodes.Type();
        }
        return res;
    }
};

template<class V>
class VToString<std::list<V>> {
public:
    std::string operator()(std::list<V> list1) {
        std::stringstream ss;
        ss << "[";
        for (int i = 0; i < list1.size() ; i++) {
            if (i != list1.size() - 1)
                ss << VToString<V>()(list1[i]) << ", ";
            else
                ss << VToString<V>()(list1[i]) << "]";
        }
        return ss.str();
    }
};

template<class V>
class StringToV<std::map<std::string, V>> {
public:
    std::map<std::string, V> operator()(const std::string& f) {
        YAML::Node nodes = YAML::Load(f);
        std::map<std::string, V> mp;
        for (auto it : nodes) {
            std::stringstream ss;
            ss << it.second;
            mp[it.first.Scalar()] = StringToV<V>()(ss.str());
        }
        return mp;
    }
};

template<class V>
class VToString<std::map<std::string, V>> {
public:
    std::string operator()(std::map<std::string, V> config) {
        std::stringstream ss;
        for (auto &[k, v] : config) {
            ss << k << ": " << VToString<V>()(v) << std::endl;
        }
        return ss.str();
    }
};

template<class V>
class StringToV<std::vector<V>> {
public:
    std::vector<V> operator()(const std::string& f) {
        YAML::Node nodes = YAML::Load(f);
        std::vector<V> res;
        if (nodes.IsSequence()) {
            for (auto it : nodes) {
                std::stringstream ss;
                ss << it;
                res.push_back(StringToV<V>()(ss.str()));
            }
        } else {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Config is not a Sequence The type is " << nodes.Type();
        }
        return res;
    }
};

template<class V>
class VToString<std::vector<V>> {
public:
    std::string operator()(std::vector<V> vec) {
        std::stringstream ss;
        ss << "[";
        for (int i = 0; i < vec.size() ; i++) {
            if (i != vec.size() - 1)
                ss << VToString<V>()(vec[i]) << ", ";
            else
                ss << VToString<V>()(vec[i]) << "]";
        }
        return ss.str();
    }
};

template<>
class StringToV<LogFormatter::ptr> {
public:
    LogFormatter::ptr operator()(const std::string& f) {
        YAML::Node nodes = YAML::Load(f);
        auto t = nodes["name"];
        LogFormatter::ptr p;
        if (t) {
            if (t.IsScalar()) {
                if (t.Scalar() == "SimpleLogFormatter") {
                    p.reset(dynamic_cast<LogFormatter *>(new SimpleLogFormatter()));
                } else if (t.Scalar() == "PatternLogFormatter") {
                    if (nodes["pattern"])
                        p.reset(dynamic_cast<LogFormatter *>(new PatternLogFormatter(nodes["pattern"].Scalar())));
                    else
                        p.reset(dynamic_cast<LogFormatter *>(new PatternLogFormatter()));
                } else {
                    D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "未知的Formatter类型";
                }
            } else {
                D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Formatter yml 格式错误";
            }
        } else {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Formatter 解析失败 无法确定Formatter类型： " << f;
        }
        return p;
    }
};

template<>
class VToString<LogFormatter::ptr> {
public:
    std::string operator()(const LogFormatter::ptr& fmt) {
        std::stringstream ss;
        ss << "\n Formatter: \n" << fmt->to_string();
        return ss.str();
    }
};
template<>
class StringToV<LogAppender::ptr> {
public:
    LogAppender::ptr operator()(const std::string& f) {
        YAML::Node nodes = YAML::Load(f);
        auto t = nodes["type"];
        LogAppender::ptr p;
        if (t) {
            if (t.IsScalar()) {
                if (t.Scalar() == "StdLogAppender") {
                    p = std::make_shared<StdLogAppender>();
                } else if (t.Scalar() == "FileLogAppender") {
                    if (nodes["filePath"]) {
                        D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << nodes["filePath"].Scalar();
                        p = std::make_shared<FileAppender>(nodes["filePath"].Scalar());
                    } else
                        p = std::make_shared<FileAppender>();
                } else {
                    D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "未知的Appender类型: " << t;
                }
            } else {
                D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Appender yml 格式错误";
            }
        } else {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Appender 解析失败 无法确定Appender类型： " << f;
        }
        if (p.get()) {
            std::stringstream ss;
            ss << nodes["formatter"];
            p->set_formatter(StringToV<LogFormatter::ptr>()(ss.str()));
        }
        return p;
    }
};

template<>
class VToString<LogAppender::ptr> {
public:
    std::string operator()(const LogAppender::ptr& apd) {
        std::stringstream ss;
        ss << "\n Formatter: \n" << apd->to_string();
        return ss.str();
    }
};


template<>
class StringToV<Logger::ptr> {
public:
    static LogLevel::Level trans_level(std::string name) {
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name == "info") {
            return LogLevel::INFO;
        } else if (name == "debug") {
            return LogLevel::DEBUG;
        } else if (name == "warn") {
            return LogLevel::WARN;
        } else if (name == "error") {
            return LogLevel::ERROR;
        } else if (name == "fatal") {
            return LogLevel::FATAL;
        } else {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Warn: can't parser LogLevel: " << name;
            return LogLevel::UNKNOWN;
        }
    }
    Logger::ptr operator()(const std::string& f) {
        YAML::Node nodes = YAML::Load(f);
        Logger::ptr p(new Logger());
        auto t = nodes["level"];
        if (t) {
            if (t.IsScalar()) {
                p->set_level(trans_level(t.Scalar()));
            }
        }
//        t = nodes["autoNewLine"];
//        if (t) {
//            if (t.IsScalar()) {
//                p->set_autoNewLine(boost::lexical_cast<bool>(t.Scalar()));
//            }
//        }
        t = nodes["appender"];
        if (t) {
            if (t.IsSequence()) {
                std::stringstream ss;
                ss << t;
                p->set_appender(StringToV<std::list<LogAppender::ptr>>()(ss.str()));
            }
        }
        return p;
    }
};

template<>
class VToString<Logger::ptr> {
public:
    std::string operator()(const Logger::ptr& logger) {
        std::string s = logger->to_string();
        return s;
    }
};

// ValueType FromStr ToStr
template<class V, class F = StringToV<V>, class T = VToString<V>>
class ConfigVar : public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar<V>> ptr;
    typedef std::function<void(V new_value, V old_value)> config_event_cb;
    ConfigVar() = default;
    ConfigVar(std::string name, std::string desc, V default_v)
                    : ConfigVarBase(name, desc), m_value(default_v) {}

    bool from_string(std::string config) override {
        auto new_value = F()(config);
        set_value(new_value);
        return true;
    }
    std::string to_string() const override {
        return T()(m_value);
    }

    std::string get_type() const override {
        return TypeToName<V>();
    }

    void set_value(V new_value) {
        for(auto &it : m_call_backs) {
            it.second(m_value, new_value);
        }
        m_value = new_value;
    }

    // 修改日志事件

    bool add_listener(uint64_t key, config_event_cb cb) {
        auto t = m_call_backs.find(key);
        if (t != m_call_backs.end()) {
            t->second = cb;
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "回调修改成功 key =" << key;
        } else {
            m_call_backs[key] = cb;
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "回调添加成功 key =" << key;
        }
        return true;
    }
    void del_listener(uint64_t key) {
        auto t = m_call_backs.find(key);
        if (t != m_call_backs.end()) {
            m_call_backs.erase(t);
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "key: " << key << "已删除";
        } else {
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "key: " << key << "不存在";
        }
    }
    void cls_listener(uint64_t key) {
        m_call_backs.clear();
    }
private:
    V m_value;
    std::map<uint64_t, config_event_cb> m_call_backs;
};

typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigMap;

class ConfigManager {
public:
    /*
     *  @TODO 需要注意的地方是，name相同但是存放的类型不同应该抛出异常或记录ERROR日志处理， 不能够直接去覆盖之前的值。
     */

    template<class V>
    typename ConfigVar<V>::ptr look_up(const std::string& name) {
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_.") != std::string::npos) {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Invalid Name";
            return nullptr;
        }
        auto it =  m_data.find(name);
        if (it != m_data.end()) {
             return std::dynamic_pointer_cast<ConfigVar<V>::ptr>(it->second);
        }
        D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "config  key: " << name << " not exist";
        return nullptr;
    }

    template<class V>
    typename ConfigVar<V>::ptr look_up(const std::string& name, const std::string& desc, const V& v) {
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_.") != std::string::npos) {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << name << " is an Invalid Name";
            return nullptr;
        }
        auto ret = m_data.find(name);
        if (ret == m_data.end()) {
            std::shared_ptr<ConfigVar<V>> res(new ConfigVar<V>(name, desc, v));
            m_data[name] = res;
            return res;
//            return std::dynamic_pointer_cast<ConfigVar<V>>(res);
        }
        return std::dynamic_pointer_cast<ConfigVar<V>>(ret->second);
    }

    ConfigVarBase::ptr look_base(const std::string& name) {
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_.") != std::string::npos) {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "Invalid Name";
            return nullptr;
        }
        auto it =  m_data.find(name);
        if (it != m_data.end()) {
            return it->second;
        }
        D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "config  key: " << name << " not exist";
        return nullptr;
    }

//    template<class V>
//    ConfigVarBase::ptr look_base(const std::string& name, const std::string& desc, V v) {
//        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_.") != std::string::npos) {
//            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << name << " is an Invalid Name";
//            return nullptr;
//        }
//        auto ret = m_data.find(name);
//        if (ret == m_data.end()) {
//            std::shared_ptr<ConfigVarBase> res(dynamic_cast<ConfigVarBase *>(new ConfigVar<V>(name, desc, v)));
//            m_data[name] = res;
//            return res;
//        }
//        return ret->second;
//    }


    void list_config() {
        for(auto &it : m_data) {
            D_SLOG_DEBUG(DREAMER_SYSTEM_LOGGER()) << "config key:" << it.first << "  value : " << it.second->to_string();
        }
    }
    ConfigMap get_data() {
        return m_data;
    }

    template<class Func>
    void loadConfig(Func f, std::string path) {
        f(path, m_data);
    }
private:
    ConfigMap m_data;
};


using ConfigMgr =  Singleton<ConfigManager> ;

#define DREAMER_ROOT_CONFIG() dreamer::ConfigMgr::getInstance()
#define DREAMER_CONFIG_DATA() DREAMER_ROOT_CONFIG()->get_data()

class YMLParser {
public:
    // 选择文件夹中的配置文件
    static int YMLFilter(const struct dirent *dir) {
        std::string p(dir->d_name);
        D_SLOG_DEBUG(DREAMER_SYSTEM_LOGGER()) << "before name :" << p;
        int pos = p.find_last_of('.');
        if (pos == std::string::npos) return 0;
        p = p.substr(pos, p.size());
        D_SLOG_DEBUG(DREAMER_SYSTEM_LOGGER()) << "after cut suffix :" << p;
        if ((p == ".yml" || p == ".yaml") && (dir->d_type & DT_REG)) return 1;
        else return 0;
    }
    void parser(YAML::Node &node, ConfigMap& configMap, std::stringstream& prefix) {
        if (node.IsNull()) {
            D_SLOG_DEBUG(DREAMER_SYSTEM_LOGGER()) << prefix.str() << node.Type() << " value is : Null" << std::endl;
        } else if (node.IsScalar()) {
            D_SLOG_DEBUG(DREAMER_SYSTEM_LOGGER()) << prefix.str() << node.Type() << " value is : " << node.Scalar() << std::endl;
        } else if (node.IsMap()) {
            auto t = DREAMER_ROOT_CONFIG()->look_base(prefix.str());
            D_SLOG_DEBUG(DREAMER_SYSTEM_LOGGER()) << prefix.str() << " value is " << node;
            if (t) {
                std::stringstream ss;
                ss << node;
                t->from_string(ss.str());
            } else {
                if (!prefix.str().empty())
                    prefix << '.';
                for (auto it : node) {
                    if (it.second.IsScalar())
                    {
                        D_SLOG_DEBUG(DREAMER_SYSTEM_LOGGER()) << prefix.str() << it.first.Scalar() << " value is : " << it.second << std::endl;
                        auto var = DREAMER_ROOT_CONFIG()->look_base(prefix.str() + it.first.Scalar());
                        if (var) {
                            var->from_string(it.second.Scalar());
                        }
                    }
                    if (it.second.IsSequence() || it.second.IsMap()) {
                        std::stringstream new_prefix;
//                        if (it.second.IsMap())
//                            new_prefix << prefix.str() << it.first.Scalar() << '.';
//                        else
//                            new_prefix << prefix.str() << it.first.Scalar();
                        new_prefix << prefix.str() << it.first.Scalar();
                        parser(it.second, configMap, new_prefix);
                    }
                }
            }
        } else if (node.IsSequence()) {
            D_SLOG_DEBUG(DREAMER_SYSTEM_LOGGER()) << prefix.str() << " value is: " << node;
            auto t = DREAMER_ROOT_CONFIG()->look_base(prefix.str());
            if (t) {
                std::stringstream ss;
                ss << node;
                t->from_string(ss.str());
            }
//            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER())  << ' ' << cnt.Type() << "   " << std::endl;
//                if (cnt.IsScalar()) {
//                    D_SLOG_INFO(DREAMER_SYSTEM_LOGGER())  << prefix.str() << " value is : " << cnt.Scalar() << std::endl;
//                    auto var = DREAMER_ROOT_CONFIG()->look_up(prefix.str());
//                    if (var) {
//                        var->from_string(cnt.Scalar());
//                    }
//                }
//                parser(cnt, configMap, prefix);
//            }
        }
    }
    bool operator()(std::string& path, ConfigMap& configMap) {
        auto type = get_type(path);
        if ( type == FileState::FILE) {
            auto config = YAML::LoadFile(path);
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "当前正在解析的配置文件为" << path ;
            std::stringstream ss;
            parser(config, configMap, ss);
        } else if (type == FileState::DIR) {
            auto ret = get_files(path, YMLFilter, alphasort);
            for(auto &i : ret) {
                auto config = YAML::LoadFile(i);
                D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "当前正在解析的配置文件为" << i ;
                std::stringstream ss;
                parser(config, configMap, ss);
            }
        } else {
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "未能发现匹配的yml配置文件";
        }

        return true;
    }
};

}

#endif //DREAMER_CONFIG_H
