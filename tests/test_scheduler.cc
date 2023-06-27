//
// Created by sly on 6/22/23.
//
#include "../sly/sly.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        sylar::Scheduler::GetThis()->schedule(&test_fiber, sylar::GetThreadId());
    }

}

void test_fiber01() {
    SYLAR_LOG_INFO(g_logger) << "test in fiber ";
    sleep(1);
    std::cout << "---------------------------------------------------------------" << std::endl;
}


void test01(){
    SYLAR_LOG_INFO(g_logger) << " main";
    sylar::Scheduler sc;
    sc.start();
    SYLAR_LOG_INFO(g_logger) << " schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << " over";
}
void test02(){
    SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc(3, false, "test");
    sc.start();
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "over";
}

void test03(){
    SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc(2);
    sc.start();
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber01);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "over";
}

int main(int argc, char** argv){
    test02();
    return 0;
}

