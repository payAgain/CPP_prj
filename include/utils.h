//
// Created by YiMing D on 2022/12/28.
//

#ifndef DREAMER_UTILS_H
#define DREAMER_UTILS_H

#include "typeinfo"
#include "cxxabi.h"
#include "log.h"
#include <execinfo.h>


namespace dreamer {

template<class T>
const char* TypeToName(){
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}

void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

template<class T, class ...Args>
inline std::shared_ptr<T> protected_make_shared(Args&&... args) {
    struct Helper : T {
        Helper(Args&&... args)
            :T(std::forward<Args>(args)...) {
        }
    };
    return std::make_shared<Helper>(std::forward<Args>(args)...);
}

}


#endif //DREAMER_UTILS_H
