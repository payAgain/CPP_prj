#include <memory>

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





}

int main()
{
//    YAML::Node config = YAML::LoadFile("/Users/yimingd/Desktop/opensource/dreamer/config/config.yaml");

//    auto test1 = config["loggers"];
//    auto d = test1["root"];
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1.Type() << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1.begin()->second << std::endl;
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

//    auto test1 = dreamer::ConfigMgr::getInstance()->look_up("system.port", "System port", 80);
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1->to_string() << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1->get_type() << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1->from_string("") << std::endl;
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1->to_string() << std::endl;

//    DREAMER_ROOT_CONFIG()->loadConfig(dreamer::YMLParser(), "/Users/yimingd/Desktop/opensource/dreamer/config/config.yaml");
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1->to_string();
//    DREAMER_ROOT_CONFIG()->list_config();

    std::vector<int> vec;
    vec.push_back(10);
    int a = 1;
    auto test1 = dreamer::ConfigMgr::getInstance()->look_up("system.ports", "System port", vec);
    dreamer::Logger::ptr p(new dreamer::Logger());
//    auto test2 = dreamer::ConfigMgr::getInstance()->look_up("loggers.root", "loggers", p);
//    std::map<std::string, dreamer::Logger::ptr> mp;
//    mp["root"] = p;
    auto test3 = dreamer::ConfigMgr::getInstance()->look_up("loggers", "loggers", dreamer::LOGGER_MAP());
    test3->add_listener(1000, dreamer::log_config_cb);
    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1->to_string();
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test2->to_string();
    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test3->to_string();
    DREAMER_ROOT_CONFIG()->loadConfig(dreamer::YMLParser(), "/Users/yimingd/Desktop/opensource/dreamer/config/config.yaml");
    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test1->to_string();
//    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test2->to_string();
    D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << test3->to_string();
    auto t = DREAMER_LOGGER_MP();
    for(auto& it : t) {
        std::cout << it.first << std::endl;
    }
//    DREAMER_ROOT_CONFIG()->list_config();
}
