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

namespace sylar{
    pid_t GetThreadId();
    uint32_t GetFiberId();


}

#endif //SLY_UTIL_H
