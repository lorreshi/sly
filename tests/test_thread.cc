//
// Created by sly on 6/16/23.
//

#include "../sly/sly.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int count = 0;
//sylar::RWMutex s_mutex;
sylar::Mutex s_mutex;

void fun1(){
    SYLAR_LOG_INFO(g_logger) << "name: " <<sylar::Thread::GetName()
                             << " this.name: " << sylar::Thread::GetThis()->getName()
                             << " id: " << sylar::GetThreadId()
                             << " this.id: " << sylar::Thread::GetThis()->getId();

    for(int i=0; i<100000; i++){
        //sylar::RWMutex::WriteLock lock(s_mutex);
        sylar::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2() {
    while(true) {
        SYLAR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while(true) {
        SYLAR_LOG_INFO(g_logger) << "========================================";
    }
}

void test(){
    std::vector<sylar::Thread::ptr> thrs;
    sylar::Thread::ptr thr1(new sylar::Thread(&fun1,"sly1"));
    sylar::Thread::ptr thr2(new sylar::Thread(&fun1,"sly2"));
    thrs.push_back(thr1);
    thrs.push_back(thr2);
    for(size_t i=0; i<thrs.size(); ++i){
        thrs[i]->join();
    }
}

void test_writefile(){
    YAML::Node root = YAML::LoadFile("/home/sly/CLionProjects/sly/cmake-build-debug/bin/log_mutex.yml");
    sylar::Config::LoadFromYaml(root);
    std::vector<sylar::Thread::ptr> thrs;
    for(int i=0; i<2; ++i){
        sylar::Thread::ptr thr(new sylar::Thread(&fun2, "name_" + std::to_string(i*2)));
        sylar::Thread::ptr thr2(new sylar::Thread(&fun3, "name_" + std::to_string(i*2+1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for(size_t i=0; i<thrs.size(); ++i){
        thrs[i]->join();
    }
}

int main(int argc, char** argv){
    SYLAR_LOG_INFO(g_logger) << "thread test begin ";

    test();
    //test_writefile();

    SYLAR_LOG_INFO(g_logger) << "thread test end ";
    SYLAR_LOG_INFO(g_logger) << "count =  " << count;




    return 0;
}
