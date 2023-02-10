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
std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");


class StringUtil {
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


    static std::string WStringToString(const std::wstring& ws);
    static std::wstring StringToWString(const std::string& s);

};

template<class T, class ...Args>
inline std::shared_ptr<T> protected_make_shared(Args&&... args) {
    struct Helper : T {
        Helper(Args&&... args)
            :T(std::forward<Args>(args)...) {
        }
    };
    return std::make_shared<Helper>(std::forward<Args>(args)...);
}

template<class Iter>
std::string MapJoin(Iter begin, Iter end, const std::string& tag1 = "=", const std::string& tag2 = "&") {
    std::stringstream ss;
    for(Iter it = begin; it != end; ++it) {
        if(it != begin) {
            ss << tag2;
        }
        ss << it->first << "=" << it->second;
    }
    return ss.str();
}

}


#endif //DREAMER_UTILS_H
