//
// Created by sly on 7/24/23.
//

#include "sly/http/http_server.h"
#include "sly/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run(){
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)){
        sleep(2);
    }
    server->start();

}

int main(int argc, char** argv){
    sylar::IOManager iom(2);
    iom.schedule(run);
    return 0;
}