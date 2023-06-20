//
// Created by sly on 6/19/23.
//
#include "../sly/sly.h"
#include <assert.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

/**
 * assert函数，为0时候返回行号并且退出
 */
void test01(){

    assert(0);
}

void test_assert(){
    SYLAR_LOG_INFO(g_logger) << sylar::BacktraceToString(10, 2, " ");
    //SYLAR_ASSERT2(0 == 1, "abcdef xx");
    SYLAR_ASSERT(false);
}

int main(int argc, char** argv){

    //test01();
    test_assert();

    return 0;
}