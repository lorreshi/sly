//
// Created by sly on 6/29/23.
//

#ifndef SLY_HOOK_H
#define SLY_HOOK_H

#include <unistd.h>

namespace sylar{

    /**
     * @brief 当前线程是否hook
     */
    bool is_hook_enable();
    /**
     * @brief 设置当前线程的hook状态
     */
    void set_hook_enable(bool flag);

}

extern "C" {

//sleep
//sleep_fun 是一个函数指针类型，它指向一个函数，该函数接受一个 unsigned int 类型的参数 seconds，
//并返回一个 unsigned int 类型的值。这种函数指针类型可以用于指向具有相同参数和返回值类型的不同函数。
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;



}

#endif //SLY_HOOK_H
