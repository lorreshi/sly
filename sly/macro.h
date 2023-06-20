//
// Created by sly on 6/19/23.
// 定义宏指令头文件
//

#ifndef SLY_MACRO_H
#define SLY_MACRO_H

#include <string.h>
#include <assert.h>
#include "util.h"
#include "log.h"


/**
 *这段代码是一个条件编译的宏定义，根据不同的编译器类型来定义SYLAR_LIKELY和SYLAR_UNLIKELY宏。
 *__GNUC__和__llvm__是编译器的预定义宏，用于标识编译器类型。在这里，如果编译器是GCC或者LLVM（Clang）
 *则使用__builtin_expect函数进行优化，否则直接使用条件表达式。 !!强制转换为bool类型
 */
#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define SYLAR_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define SYLAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define SYLAR_LIKELY(x)      (x)
#   define SYLAR_UNLIKELY(x)      (x)
#endif


/**
 * 断言宏封装
 */
#define SYLAR_ASSERT(x) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << sylar::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }


/// 断言宏封装
#define SYLAR_ASSERT2(x, w) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << sylar::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }


#endif //SLY_MACRO_H
