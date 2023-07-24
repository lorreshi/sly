//
// Created by sly on 7/23/23.
// HTTPSession封装 服务端的accept的socket叫做session，客户端的叫做connection

#ifndef SLY_HTTP_SESSION_H
#define SLY_HTTP_SESSION_H

#include "http.h"
#include "sly/socket_stream.h"

namespace sylar {
    namespace http {

        /**
         * @brief HTTPSession封装
         */
        class HttpSession : public SocketStream {
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<HttpSession> ptr;

            /**
             * @brief 构造函数
             * @param[in] sock Socket类型
             * @param[in] owner 是否托管
             */
            HttpSession(Socket::ptr sock, bool owner = true);

            /**
             * @brief 接收HTTP请求
             */
            HttpRequest::ptr recvRequest();

            /**
             * @brief 发送HTTP响应
             * @param[in] rsp HTTP响应
             * @return >0 发送成功
             *         =0 对方关闭
             *         <0 Socket异常
             */
            int sendResponse(HttpResponse::ptr rsp);
        };

    }
}

#endif //SLY_HTTP_SESSION_H
