//
// Created by YiMing D on 2022/12/27.
//

#include "config.h"
#include "boost/lexical_cast.hpp"
#include "utils.h"

namespace dreamer{


//template<class V>
//bool ConfigVar<V>::from_string(std::string config) {
//    try {
//        V m_value = boost::lexical_cast<V>(config);
//        D_SLOG_INFO(DREAMER_STD_ROOT_LOGGER()) << "转换成功 T的值为 : " << m_value << std::endl;
//    } catch (std::exception e) {
//        D_SLOG_WARN(DREAMER_STD_ROOT_LOGGER()) << "类型转换失败 : " << e.what()
//                        << "源config为 : " << config << "  目的类型: " <<  get_type() <<  std::endl;
//    }
//    return true;
//}
//
//template<class V>
//std::string ConfigVar<V>::to_string() const {
//    return boost::lexical_cast<std::string>(m_value);
//}


}