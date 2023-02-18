#include "io_manager.h"
#include "server/tcp_server.h"
#include "d_bytearray.h"
#include "log.h"
#include "worker.h"
#include "string.h"
#include "utils.h"


static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

class EchoServer : public dreamer::TcpServer {
public:
    EchoServer(int type);
    void handleClient(dreamer::Socket::ptr client);

private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    :m_type(type) {
}

void EchoServer::handleClient(dreamer::Socket::ptr client) {
    D_SLOG_INFO(g_logger) << "handleClient " << *client;   
    // dreamer::ByteArray::ptr ba(new dreamer::ByteArray);
    while(true) {
        // ba->clear();
        char *p = new char[100];
        // std::vector<iovec> iovs;
        // ba->getWriteBuffers(iovs, 1024);

        // int rt = client->recv(&iovs[0], iovs.size());
        int rt = client->recv(p, 100);
        if(rt == 0) {
            D_SLOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if(rt < 0) {
            D_SLOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        // ba->resetPos(ba->getPos() + rt);
        // ba->resetPos(0);
        //D_SLOG_INFO(g_logger) << "recv rt=" << rt << " data=" << std::string((char*)iovs[0].iov_base, rt);
        if(m_type == 1) {//text 
            std::cout << p;// << std::endl;
        } else {
            // std::cout << ba->toHexString();// << std::endl;
        }
        delete []p;
        //std::string tmp = "HTTP/1.1 100 Continue\r\n\r\n";
        //client->send(tmp.c_str(), tmp.size());
        std::cout.flush();
    }
}

int type = 1;

void test() {
    D_SLOG_INFO(g_logger) << "=========== test begin";
    dreamer::TimeCalc tc;
    dreamer::TimedWorkerGroup::ptr wg = dreamer::TimedWorkerGroup::Create(5, 1000);
    for(size_t i = 0; i < 10; ++i) {
        wg->schedule([i](){
            sleep(i);
            D_SLOG_INFO(g_logger) << "=========== " << i;
        });
    }
    wg->waitAll();
    D_SLOG_INFO(g_logger) << "=========== " << tc.elapse() << " over";
}

void run() {
    D_SLOG_INFO(g_logger) << "server type=" << type;
    EchoServer::ptr es(new EchoServer(type));
    auto addr = dreamer::Address::LookupAny("0.0.0.0:8020");
    while(!es->bind(addr)) {
        sleep(2);
    }
    es->start();

    dreamer::IOManager::GetThis()->schedule(test);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        D_SLOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if(!strcmp(argv[1], "-b")) {
        type = 2;
    }

    dreamer::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
