//
// Created by sly on 6/3/23.
//

#include "util.h"
#include "log.h"
#include <execinfo.h>
#include "fiber.h"

namespace sylar{

    //自己出问题的日志都设置为system
    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }
    uint32_t GetFiberId(){
        return sylar::Fiber::GetFiberId();
    }

    void Backtrace(std::vector<std::string>& bt, int size, int skip){
        //不用指针的方式创建，指针占用占空间内存
        void** array = (void**) malloc((sizeof(void*)* size));
        // backtrace 2个函数 1.
        size_t s = ::backtrace(array, size);
        // 2.
        char** strings = backtrace_symbols(array, s);

        if(strings == NULL){
            SYLAR_LOG_ERROR(g_logger) << "backtrace_synbols error! ";
            return;
        }
        // 调用栈保存在vector
        for(size_t i=skip; i<s; ++i){
            bt.push_back(strings[i]);
        }

        free(strings);
        free(array);

    }

    std::string BacktraceToString(int size, int skip, const std::string& prefix){
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for(size_t i = 0; i < bt.size(); ++i) {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }

}