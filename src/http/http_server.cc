#include "http/http_server.h"
#include "log.h"
#include "http/http_session.h"

namespace dreamer {
namespace http {

static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

HttpServer::HttpServer(bool keepalive
               ,IOManager* worker
               ,IOManager* io_worker
               ,IOManager* accept_worker) 
                : TcpServer(worker, io_worker, accept_worker)
                , m_isKeepalive(keepalive) {
    m_dispatch = std::make_shared<ServletDispatch>();
    m_dispatch->addServlet("/", std::make_shared<NotFoundServlet>("NotFind"));
    m_type = "http"; 
}

void HttpServer::handleClient(Socket::ptr client) {
    D_SLOG_DEBUG(g_logger) << "handleClient " << *client;
    //TimeCalc tc;
    HttpSession::ptr session = std::make_shared<HttpSession>(client);
    do {
        auto req = session->recvRequest();
        //tc.tick("recv");
        if(!req) {
            D_SLOG_DEBUG(g_logger) << "recv http request fail, errno="
                << errno << " errstr=" << strerror(errno)
                << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
            break;
        }

        HttpResponse::ptr rsp = std::make_shared<HttpResponse>(req->getVersion()
                            ,req->isClose() || !m_isKeepalive);
        rsp->setHeader("Server", getName());
        rsp->setHeader("Content-Type", "application/json;charset=utf8");
        // {
        //     SchedulerSwitcher sw(m_worker);
        //     m_dispatch->handle(req, rsp, session);
        // }
        m_dispatch->handle(req, rsp, session);
        //tc.tick("handler");
        session->sendResponse(rsp);
        //tc.tick("response");
        // D_SLOG_ERROR(g_logger) << "elapse=" << tc.elapse() << " - " << tc.toString();
        D_SLOG_DEBUG(g_logger) << "rsp" << rsp->toString();

        if(!m_isKeepalive || req->isClose()) {
            break;
        }
    } while(true);
    session->close();
}

}
}