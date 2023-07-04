//
// Created by sly on 6/27/23.
//
#include "../sly/sly.h"
#include "sly/iomanager.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>


sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
//定义一个socket测试
int sock = 0;

void test_fiber(){
    SYLAR_LOG_INFO(g_logger) << " test fiber sock = " << sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    std::cout << "rt: " << rt << std::endl;

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {

    } else if(errno == EINPROGRESS) {
        SYLAR_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::READ, [](){
            SYLAR_LOG_INFO(g_logger) << "read callback";
        });
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::WRITE, [](){
            SYLAR_LOG_INFO(g_logger) << "write callback";
            close(sock);
            sylar::IOManager::GetThis()->cancelEvent(sock, sylar::IOManager::READ);
            close(sock);
        });
    } else {
        SYLAR_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

void test01(){
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    sylar::IOManager iom(2, false,"test");
    iom.schedule(&test_fiber);
}

//sylar::Timer::ptr s_timer;
void test_timer() {
    sylar::IOManager iom(2);
    iom.addTimer(1000, [](){
        SYLAR_LOG_INFO(g_logger) << "hello timer i=";


    }, true);
}

void test_timer01() {
    sylar::IOManager iom(2);
    sylar::Timer::ptr timer = iom.addTimer(1000, [&timer](){
        static int i = 0;
        SYLAR_LOG_INFO(g_logger) << "hello timer i= " << i;
        if(++i == 3){
            timer->cancel();
        }

    }, true);
}

void test_timer02() {
    sylar::IOManager iom(2);
    sylar::Timer::ptr timer = iom.addTimer(1000, [&timer](){
        static int i = 0;
        SYLAR_LOG_INFO(g_logger) << "hello timer i= " << i;
        if(++i == 6){
            timer->reset(2000, true);
        }

    }, true);
}


int main(int argc, char** argv){
    test01();
    //test_timer01();
    //test_timer02();
    return 0;
}