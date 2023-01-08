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
            D_SLOG_WARN(DREAMER_STD_ROOT_LOGGER()) << "Config is not a Sequence The type is " << nodes.Type();
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

// ValueType FromStr ToStr
template<class V, class F = StringToV<V>, class T = VToString<V>>
class ConfigVar : public ConfigVarBase{
public:
    ConfigVar() = default;
    ConfigVar(std::string name, std::string desc, V default_v)
                    : ConfigVarBase(name, desc), m_value(default_v) {}

    bool from_string(std::string config) override {
        m_value = F()(config);
        return true;
    }
    std::string to_string() const override {
        return T()(m_value);
    }

    std::string get_type() const override {
        return TypeToName<V>();
    }

//    static ConfigVarBase::ptr get_instance(std::string name, std::string desc, V v) {
//        std::shared_ptr<ConfigVarBase> res;
//        res.reset((ConfigVarBase *)new ConfigVar(name, desc, v));
//        res.reset(dynamic_cast<ConfigVarBase*>(new ConfigVar(name, desc, v)));
//        return res;
//    }

private:
    V m_value;
};

typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigMap;

class ConfigManager {
public:
    /*
     *  @TODO 需要注意的地方是，name相同但是存放的类型不同应该抛出异常或记录ERROR日志处理， 不能够直接去覆盖之前的值。
     */
    ConfigVarBase::ptr look_up(const std::string& name) {
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_.") != std::string::npos) {
            D_SLOG_WARN(DREAMER_STD_ROOT_LOGGER()) << "Invalid Name";
            return nullptr;
        }
        auto it =  m_data.find(name);
        if (it != m_data.end()) {
             return it->second;
        }
        D_SLOG_WARN(DREAMER_STD_ROOT_LOGGER()) << "config  key: " << name << " not exist";
        return nullptr;
    }

    template<class V>
    ConfigVarBase::ptr look_up(const std::string& name, const std::string& desc, V v) {
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789_.") != std::string::npos) {
            D_SLOG_WARN(DREAMER_STD_ROOT_LOGGER()) << name << " is an Invalid Name";
            return nullptr;
        }
        auto ret = m_data.find(name);
        if (ret == m_data.end()) {
            std::shared_ptr<ConfigVarBase> res(dynamic_cast<ConfigVarBase *>(new ConfigVar<V>(name, desc, v)));
            m_data[name] = res;
            return res;
        }
        return ret->second;
    }

    void list_config() {
        for(auto &it : m_data) {
            D_SLOG_DEBUG(DREAMER_STD_ROOT_LOGGER()) << "config key:" << it.first << "  value : " << it.second->to_string();
        }
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

class YMLParser {
public:
    // 选择文件夹中的配置文件
    static int YMLFilter(const struct dirent *dir) {
        std::string p(dir->d_name);
        D_SLOG_DEBUG(DREAMER_STD_ROOT_LOGGER()) << "before name :" << p;
        int pos = p.find_last_of('.');
        if (pos == std::string::npos) return 0;
        p = p.substr(pos, p.size());
        D_SLOG_DEBUG(DREAMER_STD_ROOT_LOGGER()) << "after cut suffix :" << p;
        if ((p == ".yml" || p == ".yaml") && (dir->d_type & DT_REG)) return 1;
        else return 0;
    }
    void parser(YAML::Node &node, ConfigMap& configMap, std::stringstream& prefix) {
        if (node.IsNull()) {
            D_SLOG_DEBUG(DREAMER_STD_ROOT_LOGGER())  << prefix.str() << node.Type() << " value is : Null" << std::endl;
        } else if (node.IsScalar()) {
            D_SLOG_DEBUG(DREAMER_STD_ROOT_LOGGER())  << prefix.str() << node.Type() << " value is : " <<  node.Scalar() << std::endl;
        } else if (node.IsMap()) {
            auto t = DREAMER_ROOT_CONFIG()->look_up(prefix.str());
            D_SLOG_DEBUG(DREAMER_STD_ROOT_LOGGER()) << prefix.str() << " value is " << node;
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
                        D_SLOG_DEBUG(DREAMER_STD_ROOT_LOGGER())  << prefix.str() << it.first.Scalar() << " value is : " << it.second << std::endl;
                        auto var = DREAMER_ROOT_CONFIG()->look_up(prefix.str() + it.first.Scalar());
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
            D_SLOG_DEBUG(DREAMER_STD_ROOT_LOGGER()) << prefix.str() << " value is: " << node;
            auto t = DREAMER_ROOT_CONFIG()->look_up(prefix.str());
            if (t) {
                std::stringstream ss;
                ss << node;
                t->from_string(ss.str());
            }
//            D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER())  << ' ' << i.Type() << "   " << std::endl;
//                if (i.IsScalar()) {
//                    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER())  << prefix.str() << " value is : " << i.Scalar() << std::endl;
//                    auto var = DREAMER_ROOT_CONFIG()->look_up(prefix.str());
//                    if (var) {
//                        var->from_string(i.Scalar());
//                    }
//                }
//                parser(i, configMap, prefix);
//            }
        }
    }
    bool operator()(std::string& path, ConfigMap& configMap) {
        auto type = get_type(path);
        if ( type == FileState::FILE) {
            auto config = YAML::LoadFile(path);
            D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << "当前正在解析的配置文件为" << path ;
            std::stringstream ss;
            parser(config, configMap, ss);
        } else if (type == FileState::DIR) {
            auto ret = get_files(path, YMLFilter, alphasort);
            for(auto &i : ret) {
                auto config = YAML::LoadFile(i);
                D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << "当前正在解析的配置文件为" << i ;
                std::stringstream ss;
                parser(config, configMap, ss);
            }
        } else {
            D_SLOG_WARN(DREAMER_STD_ROOT_LOGGER()) << "未能发现匹配的yml配置文件";
        }

        return true;
    }
};

}

#endif //DREAMER_CONFIG_H
