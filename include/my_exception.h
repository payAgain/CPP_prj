//
// Created by YiMing D on 2023/1/17.
//

#ifndef DREAMER_MY_EXCEPTION_H
#define DREAMER_MY_EXCEPTION_H

#include "utils.h"
#include "log.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define DREAMER_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define DREAMER_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define DREAMER_LIKELY(x)      (x)
#   define DREAMER_UNLIKELY(x)      (x)
#endif

/// 断言宏封装
#define DREAMER_ASSERT(x) \
    if(DREAMER_UNLIKELY(!(x))) { \
        D_SLOG_ERROR(DREAMER_SYSTEM_LOGGER()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << dreamer::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

/// 断言宏封装
#define DREAMER_ASSERT2(x, w) \
    if(DREAMER_UNLIKELY(!(x))) { \
        D_SLOG_ERROR(DREAMER_SYSTEM_LOGGER()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << dreamer::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }



#endif //DREAMER_MY_EXCEPTION_H
