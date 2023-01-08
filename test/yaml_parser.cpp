#include "yaml-cpp/yaml.h"
#include "log.h"
#include "config.h"


void parser_yml(YAML::Node &node, int l) {
    if (node.IsNull()) {
        D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << l << ' ' << node.Type() << " value is : Null" << std::endl;
    } else if (node.IsScalar()) {
        D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << l << ' ' << node.Type() << " value is : " <<  node.Scalar() << std::endl;
    } else if (node.IsMap()) {
//        D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << node;
        for (auto it : node) {
//            D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << it.first << " value is : " << it.second << std::endl;
//            D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << it;
            if (it.second.IsSequence() || it.second.IsMap())
                parser_yml(it.second, l + 1);
        }
    } else if (node.IsSequence()) {
        D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << "This is a Sequence" << node << std::endl;
        for(auto && i : node) {
//            D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << << i << "   " << std::endl;
            parser_yml(i, l + 1);
        }
    }
}

namespace dreamer {
//    template<class V>
//    class StringToV<std::vector<V>> {
//    public:
//        std::vector<V> operator()(const std::string& f) {
//            YAML::Node nodes = YAML::Load(f);
//            std::vector<V> res;
//            if (nodes.IsSequence()) {
//                for (auto it : nodes) {
//                    std::stringstream ss;
//                    ss << it;
//                    res.push_back(StringToV<V>()(ss.str()));
//                }
//            } else {
//                D_SLOG_WARN(DREAMER_STD_ROOT_LOGGER()) << "Config is not a Sequence The type is " << nodes.Type();
//            }
//            return res;
//        }
//    };
//
//    template<class V>
//    class VToString<std::vector<V>> {
//    public:
//        std::string operator()(std::vector<V> vec) {
//            std::stringstream ss;
//            ss << "[";
//            for (int i = 0; i < vec.size() ; i++) {
//                if (i != vec.size() - 1)
//                    ss << VToString<V>()(vec[i]) << ", ";
//                else
//                    ss << VToString<V>()(vec[i]) << "]";
//            }
//            return ss.str();
//        }
//    };
}

int main()
{
//    YAML::Node config = YAML::LoadFile("/Users/yimingd/Desktop/opensource/dreamer/config/config.yaml");

//    auto t = config["loggers"];
//    auto d = t["root"];
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t.Type() << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t.begin()->second << std::endl;
//    parser_yml(config, 1);

//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << d.Type() << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << d.size() << std::endl;
//    if (dreamer::LogMgr::getInstance()->get_std_root_logger()->get_level() <= dreamer::LogLevel::Level::INFO)
//        dreamer::LogEventWrap(dreamer::LogMgr::getInstance()->get_std_root_logger(), "_file_name_", 7,
//                              dreamer::LogLevel::Level::INFO).get_ss() << config;
//    if (dreamer::LogMgr::getInstance()->get_std_root_logger()->get_level() <= dreamer::LogLevel::Level::INFO)
//        dreamer::LogEventWrap(dreamer::LogMgr::getInstance()->get_std_root_logger(), "_file_name_", 7,
//                              dreamer::LogLevel::Level::INFO).get_ss() << config;
//    if (config["lastLogin"]) {
//        std::cout << "Last logged in: " << config["lastLogin"].as<DateTime>() << "\n";
//    }

//    std::string a = "ABD";
//    std::transform(a.begin(), a.end() , a.begin(), ::tolower);
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << a << std::endl;

//    auto t = dreamer::ConfigMgr::getInstance()->look_up("system.port", "System port", 80);
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t->to_string() << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t->get_type() << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t->from_string("") << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t->to_string() << std::endl;

//    DREAMER_ROOT_CONFIG()->loadConfig(dreamer::YMLParser(), "/Users/yimingd/Desktop/opensource/dreamer/config/config.yaml");
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t->to_string();
//    DREAMER_ROOT_CONFIG()->list_config();

    std::vector<int> vec;
    vec.push_back(10);
    auto t = dreamer::ConfigMgr::getInstance()->look_up("system.ports", "System port", vec);
    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t->to_string();
    DREAMER_ROOT_CONFIG()->loadConfig(dreamer::YMLParser(), "/Users/yimingd/Desktop/opensource/dreamer/config/config.yaml");
    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << t->to_string();
    DREAMER_ROOT_CONFIG()->list_config();
}
