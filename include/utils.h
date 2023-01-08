//
// Created by YiMing D on 2022/12/28.
//

#ifndef DREAMER_UTILS_H
#define DREAMER_UTILS_H

#include "typeinfo"
#include "cxxabi.h"

namespace dreamer {

template<class T>
const char* TypeToName() {
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}

}


#endif //DREAMER_UTILS_H
