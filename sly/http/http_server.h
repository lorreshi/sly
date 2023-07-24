//
// Created by sly on 7/23/23.
//

#ifndef SLY_HTTP_SERVER_H
#define SLY_HTTP_SERVER_H

#include "sly/tcp_server.h"
#include "http_session.h"

namespace sylar {
    namespace http {

        /**
         * @brief HTTP服务器类
         */
        class HttpServer : public TcpServer {
        public:
            /// 智能指针类型
            typedef std::shared_ptr<HttpServer> ptr;

            /**
             * @brief 构造函数
             * @param[in] keepalive 是否长连接
             * @param[in] worker 工作调度器
             * @param[in] accept_worker 接收连接调度器
             */
            HttpServer(bool keepalive = false
                    ,sylar::IOManager* worker = sylar::IOManager::GetThis()
                    ,sylar::IOManager* accept_worker = sylar::IOManager::GetThis());

        protected:
            virtual void handleClient(Socket::ptr client) override;
        private:
            /// 是否支持长连接
            bool m_isKeepalive;

        };

    }
}


#endif //SLY_HTTP_SERVER_H
