#include "stream/d_stream.h"
#include "log.h"
#include "config.h"

namespace dreamer {

static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

static ConfigVar<int32_t>::ptr g_socket_buff_size =
    ConfigMgr::getInstance()->look_up("socket.buff_size"
                , "socket buff size", (int32_t)(1024 * 16));

int Stream::readFixSize(void* buffer, size_t length) {
    size_t offset = 0;
    int64_t left = length;
    static const int64_t MAX_LEN = g_socket_buff_size->get_value();
    while(left > 0) {
        int64_t len = read((char*)buffer + offset, std::min(left, MAX_LEN));
        if(len <= 0) {
            D_SLOG_ERROR(g_logger) << "readFixSize fail length=" << length
                << " len=" << len << " errno=" << errno << " errstr=" << strerror(errno);
            return len;
        }
        offset += len;
        left -= len;
    }
    return length;
}

int Stream::readFixSize(ByteArray::ptr ba, size_t length) {
    int64_t left = length;
    static const int64_t MAX_LEN = g_socket_buff_size->get_value();
    while(left > 0) {
        int64_t len = read(ba, std::min(left, MAX_LEN));
        if(len <= 0) {
            D_SLOG_ERROR(g_logger) << "readFixSize fail length=" << length
                << " len=" << len << " errno=" << errno << " errstr=" << strerror(errno);
            return len;
        }
        left -= len;
    }
    return length;
}

int Stream::writeFixSize(const void* buffer, size_t length) {
    size_t offset = 0;
    int64_t left = length;
    static const int64_t MAX_LEN = g_socket_buff_size->get_value();
    while(left > 0) {
        int64_t len = write((const char*)buffer + offset, std::min(left, MAX_LEN));
        //int64_t len = write((const char*)buffer + offset, left);
        if(len <= 0) {
            D_SLOG_ERROR(g_logger) << "writeFixSize fail length=" << length << " len=" << len
                << " left=" << left << " errno=" << errno << ", " << strerror(errno);
            return len;
        }
        offset += len;
        left -= len;
    }
    return length;

}

int Stream::writeFixSize(ByteArray::ptr ba, size_t length) {
    int64_t left = length;
    while(left > 0) {
        static const int64_t MAX_LEN = g_socket_buff_size->get_value();
        int64_t len = write(ba, std::min(left, MAX_LEN));
        if(len <= 0) {
            D_SLOG_ERROR(g_logger) << "writeFixSize fail length=" << length << " len=" << len
                << " errno=" << errno << ", " << strerror(errno);
            return len;
        }
        left -= len;
    }
    return length;
}

}