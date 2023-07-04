//
// Created by sly on 7/3/23.
//
#include "sly/address.h"
#include "sly/log.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

// 转换
void test() {
    std::vector<sylar::Address::ptr> addrs;

    SYLAR_LOG_INFO(g_logger) << "begin";
    //bool v = sylar::Address::Lookup(addrs, "localhost:3080");
    bool v = sylar::Address::Lookup(addrs, "www.baidu.com");
    //bool v = sylar::Address::Lookup(addrs, "www.sylar.top", AF_INET);
    SYLAR_LOG_INFO(g_logger) << "end";
    if(!v) {
        SYLAR_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        SYLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

}

void test_iface() {
    std::multimap<std::string, std::pair<sylar::Address::ptr, uint32_t> > results;

    bool v = sylar::Address::GetInterfaceAddresses(results);
    if(!v) {
        SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        SYLAR_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
                                 << i.second.second;
    }
}

void test_ipv4() {
    //auto addr = sylar::IPAddress::Create("www.sylar.top");
    auto addr = sylar::IPAddress::Create("127.0.0.8");
    if(addr) {
        SYLAR_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc, char** argv) {

    //test();
    //test_iface();
    test_ipv4();

    return 0;
}