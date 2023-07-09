//
// Created by sly on 7/8/23.
//
#include "../sly/http/http.h"
#include "../sly/log.h"

void test_request() {
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setHeader("host" , "www.baidu.top");
    req->setBody("hello world");
    req->dump(std::cout) << std::endl;
}

void test_response() {
    sylar::http::HttpResponse::ptr rsp(new sylar::http::HttpResponse);
    rsp->setHeader("X-X", "sly");
    rsp->setBody("hello world");
    rsp->setStatus((sylar::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
    test_request();
    test_response();
    return 0;
}