//
// Created by sly on 6/3/23.
//

#ifndef SLY_UTIL_H
#define SLY_UTIL_H

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <zconf.h>   //syscall 函数
#include <stdio.h>
#include <cstdint>   // uint32_t
#include <vector>
#include <string>

namespace sylar{
    pid_t GetThreadId();
    uint32_t GetFiberId();

    /**
     * 从新封装backtrace函数，
     * @param bt 调用栈保存在vector
     * @param size 返回层数
     * @param skip 跳过栈顶层数
     */
    void Backtrace(std::vector<std::string>& bt, int size, int skip);

    /**
 * @brief 获取当前栈信息的字符串
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

}

#endif //SLY_UTIL_H
