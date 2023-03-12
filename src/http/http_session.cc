#include "basic_log.h"
#include "http/d_http_parser.h"
#include "http/http_session.h"
#include "string.h"
#include "log.h"

namespace dreamer {
namespace http {
static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

HttpSession::HttpSession(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner) {
}

HttpRequest::ptr HttpSession::recvRequest() {
    HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    std::shared_ptr<char> buffer(
            new char[buff_size], [](char* ptr){
                delete[] ptr;
            });
    char* data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset); // 最多读取buff_size的数据
        D_SLOG_DEBUG(g_logger) << "read data: " << data;
        if(len <= 0) {
            close();
            D_SLOG_INFO(g_logger) << "client error rt=" << len 
                << " errno=" << errno << " errstr=" << strerror(errno);
            return nullptr;
        } 
        len += offset;
        size_t nparse = parser->execute(data, len); 
        if(auto t = parser->hasError()) {
            D_SLOG_INFO(g_logger) << "parser error rt= " << t
                << " errno=" << errno << " errstr=" << strerror(errno);
            close();
            return nullptr;
        }
        offset = len - nparse; 
        if(offset == (int)buff_size) {
            close();
            return nullptr;
        }
        if(parser->isFinished()) {
            break;
        }
    } while(true);
    int64_t length = parser->getContentLength();

    auto v = parser->getData()->getHeader("Expect");
    if(strcasecmp(v.c_str(), "100-continue") == 0) {
        static const std::string s_data = "HTTP/1.1 100 Continue\r\n\r\n";
        writeFixSize(s_data.c_str(), s_data.size());
        parser->getData()->delHeader("Expect");
    }

    if(length > 0) {
        std::string body;
        body.resize(length);

        int len = 0;
        if(length >= offset) {
            memcpy(&body[0], data, offset);
            len = offset;
        } else {
            memcpy(&body[0], data, length);
            len = length;
        }
        length -= offset;
        if(length > 0) {
            if(readFixSize(&body[len], length) <= 0) {
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }

    parser->getData()->init();
    return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}


}
}