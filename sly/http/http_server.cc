//
// Created by sly on 7/23/23.
//
#include "http_server.h"
#include "sly/log.h"

namespace sylar {
    namespace http {

        static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

        HttpServer::HttpServer(bool keepalive
                ,sylar::IOManager* worker
                ,sylar::IOManager* accept_worker)
                :TcpServer(worker, accept_worker)
                ,m_isKeepalive(keepalive)
                {
                    m_dispatch.reset(new ServletDispatch);
        }


        void HttpServer::handleClient(Socket::ptr client) {
            SYLAR_LOG_DEBUG(g_logger) << "handleClient " << *client;
            HttpSession::ptr session(new HttpSession(client));
            do {
                auto req = session->recvRequest();
                if(!req) {
                    SYLAR_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                                              << errno << " errstr=" << strerror(errno)
                                              << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
                    break;
                }

                HttpResponse::ptr rsp(new HttpResponse(req->getVersion()
                        ,req->isClose() || !m_isKeepalive));

                m_dispatch->handle(req, rsp, session);

//                rsp->setBody("Hello sly");
//
//                SYLAR_LOG_INFO(g_logger) << "requst: " << std::endl << *req;
//                SYLAR_LOG_INFO(g_logger) << "response: " << std::endl << *rsp;

                session->sendResponse(rsp);

            } while(true);
            session->close();
        }

    }
}
