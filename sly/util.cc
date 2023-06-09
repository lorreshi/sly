//
// Created by sly on 6/3/23.
//

#include "util.h"

namespace sylar{
    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }
    uint32_t GetFiberId(){
        return 0;
    }

}