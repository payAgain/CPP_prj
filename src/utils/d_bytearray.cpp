#include "d_bytearray.h"
#include "d_exception.h"
#include "file_util.h"
#include "log.h"
#include <endian.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace dreamer {

static Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();

ByteArray::Node::Node(size_t s) 
            : size(s)
            , next(nullptr)
            , ptr(new char[s]){
    
}
ByteArray::Node::~Node() {
    if (ptr)
        freeNode();
    next = nullptr;
    D_SLOG_DEBUG(g_logger) << "Node memory release";
}
void ByteArray::Node::freeNode() {
    delete [] ptr;
}

ByteArray::ByteArray(size_t base_size)
                : m_baseSize(base_size)
                , m_position(0)
                , m_size(base_size)
                , m_root(new Node(base_size))
                , m_cur(m_root)
                , m_eof(0)
                , m_capacity(base_size) {

}
ByteArray::~ByteArray() {
    Node* tmp = m_root;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
}

// class interface
void ByteArray::resize(size_t sizeNeed) {
    if (DREAMER_UNLIKELY(sizeNeed <= 0)) {
        return;
    }
    Node* tmp = m_root;
    int cnt = 0;
    while(tmp->next) { tmp = tmp->next; }
    while(sizeNeed) {
        Node* newNode = new Node(m_baseSize);
        tmp->next = newNode;
        tmp = tmp->next;
        if (sizeNeed > m_baseSize)
            sizeNeed -= m_baseSize;
        else
            sizeNeed = 0;
        m_capacity += m_baseSize;
        cnt++;
    }
    D_SLOG_INFO(g_logger) << "ByteArray capacity increase: " << cnt * m_baseSize;
}
void ByteArray::resetPos(size_t p) {
    m_position = p;
    if (p > m_eof) {
        throw std::out_of_range("out range of eof");
    }
    size_t cnt = m_position / m_baseSize;
    m_cur = m_root;
    while(cnt--) {
        m_cur = m_cur->next;
    } 
}
std::string ByteArray::toString() {
    std::string res;
    res.resize(m_eof);
    auto prePos = getPos();
    resetPos(0);
    readFromArray(&res[0], res.size());
    resetPos(prePos);
    return res;
}
void ByteArray::writeToArray(const void* buff, size_t buffSize) {
    size_t writedBuff = 0;
    while(buffSize) {
        size_t curNodeSize = m_position % m_baseSize;
        size_t leftNodeSize = m_baseSize - curNodeSize;
        if (leftNodeSize >= buffSize) {
            D_SLOG_DEBUG(g_logger) << "insertPtr: " << m_cur << " insertPos: "
                                   << curNodeSize << " write bufferSize: " << buffSize;
            memcpy(m_cur->ptr + curNodeSize, (char *)buff + writedBuff, buffSize);
            m_position += buffSize;
            buffSize = 0;
        } else {
            D_SLOG_DEBUG(g_logger) << "insertPtr: " << m_cur << " insertPos: "
                                   << curNodeSize << " write bufferSize: " << leftNodeSize;
            memcpy(m_cur->ptr + curNodeSize, (char *)buff + writedBuff, leftNodeSize);
            m_position += leftNodeSize;
            buffSize -= leftNodeSize;
            writedBuff += leftNodeSize;
        }
        if (m_position && m_position % m_baseSize == 0) {
            if (!m_cur->next) {
                resize(buffSize + 1);
            }
            m_cur = m_cur->next;
        }
    }
    if (m_position >= m_eof) m_eof = m_position; 
    D_SLOG_DEBUG(g_logger) << "all data write to Array";
}
void ByteArray::readFromArray(void* buff, size_t buffSize) {
    if (buffSize > m_capacity - m_position) {
        throw std::out_of_range("out of buffer range");
    }
    size_t cnt = 0; // buff head pointer
    while(buffSize) {
        int curNodeSize = m_position % m_baseSize;
        int leftNodeSize = m_baseSize - curNodeSize;
        if (leftNodeSize >= buffSize) {
            D_SLOG_DEBUG(g_logger) << "insertPtr: " << m_cur << " insertPos: "
                                   << curNodeSize << " read bufferSize: " << buffSize;
            memcpy(((char *)buff + cnt), m_cur->ptr + curNodeSize, buffSize);
            m_position += buffSize;
            buffSize = 0;
        } else {
            D_SLOG_DEBUG(g_logger) << "insertPtr: " << m_cur << " insertPos: "
                                   << curNodeSize << " read bufferSize: " << leftNodeSize;
            memcpy(((char *)buff + cnt), m_cur->ptr + curNodeSize, leftNodeSize);
            buffSize -= leftNodeSize;
            m_position += leftNodeSize;
            cnt += leftNodeSize;
        }
        if (m_position && m_position % m_baseSize == 0) {
            m_cur = m_cur->next;
        }
        // if (m_position >= m_eof) m_eof = m_position; 
    }
}
void ByteArray::writeToFile(const std::string& fileName, size_t buffSize) {
    if (buffSize == (size_t)-1) {
        buffSize = m_eof - m_position;
    }
    if (buffSize > m_eof - m_position) {
        throw std::out_of_range("out range of buffSize");
    }
    FileOperation fop;
    fop.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    while(buffSize) {
        int curNodeSize = m_position % m_baseSize;
        int leftSize = m_baseSize - curNodeSize;
        if (leftSize >= buffSize) {
            fop.write(m_cur->ptr + curNodeSize, buffSize);
            m_position += buffSize;
            buffSize = 0;
        } else {
            fop.write(m_cur->ptr + curNodeSize, leftSize);
            buffSize -= leftSize;
            m_position += leftSize;
        }
        if (m_position && m_position % m_baseSize == 0) {
            m_cur = m_cur->next;
        }
    }
}
void ByteArray::readFromFile(const std::string& fileName, size_t buffSize) {
    int leftSize = m_capacity - m_position;
    FileOperation fop;
    fop.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    buffSize = (buffSize == (size_t)-1) ? GetFileSize(fileName) : buffSize; 
    if (buffSize > leftSize) {
        resize(buffSize - leftSize);
    }
    while(buffSize) {
        int curNodePos = m_position % m_baseSize;
        int nodeLeftSize = m_baseSize - curNodePos;
        if (buffSize <= nodeLeftSize) {
            fop.read(m_cur->ptr + curNodePos, buffSize);
            m_position += buffSize;
            buffSize = 0;
        } else {
            fop.read(m_cur->ptr + curNodePos, nodeLeftSize);
            buffSize -= nodeLeftSize;
            m_position += nodeLeftSize;
        }
        if (m_position && m_position % m_baseSize == 0) {
            m_cur = m_cur->next;
        }
    }

}

// write./
void ByteArray::writeFloat(float v) {
    uint32_t t;
    memcpy(&t, &v, sizeof(v));
    writeFixInt(t);
}
void ByteArray::writeDouble(double v) {
    uint32_t t;
    memcpy(&t, &v, sizeof(v));
    writeFixInt(t);
}
static uint32_t EncodeZigzag32(const int32_t& v) {
    if(v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}
static uint64_t EncodeZigzag64(const int64_t& v) {
    if(v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}
static int32_t DecodeZigzag32(const uint32_t& v) {
    return (v >> 1) ^ -(v & 1);
}
static int64_t DecodeZigzag64(const uint64_t& v) {
    return (v >> 1) ^ -(v & 1);
}
void ByteArray::writeInt32  (int32_t v) {
    writeUint32(EncodeZigzag32(v));
}
void ByteArray::writeUint32 (uint32_t v) {
    uint8_t tmp[5];
    uint8_t i = 0;
    while(v >= 0x80) {
        tmp[i++] = (v & 0x7F) | 0x80;
        v >>= 7;
    }
    tmp[i++] = v;
    writeToArray(tmp, i);
}
void ByteArray::writeInt64  (int64_t v) {
    writeUint64(EncodeZigzag64(v));
}
void ByteArray::writeUint64 (uint64_t v) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while(v >= 0x80) {
        tmp[i++] = (v & 0x7F) | 0x80;
        v >>= 7;
    }
    tmp[i++] = v;
    writeToArray(tmp, i);
}
void ByteArray::writeStringWithFixLength(const std::string& v) {
    writeFixInt(v.size());
    writeToArray(v.c_str(), v.size());
}
void ByteArray::writeStringWithUnfixLength(const std::string& v) {
    writeUint64(v.size());
    writeToArray(v.c_str(), v.size());
}

// read
int32_t  ByteArray::readInt32() {
    return DecodeZigzag32(readUint32());
}
uint32_t ByteArray::readUint32() {
    uint32_t result = 0;
    for(int i = 0; i < 32; i += 7) {
        uint8_t b;
        readFixInt(b);
        if(b < 0x80) {
            result |= ((uint32_t)b) << i;
            break;
        } else {
            result |= (((uint32_t)(b & 0x7f)) << i);
        }
    }
    return result;
}
int64_t  ByteArray::readInt64() {
    return DecodeZigzag64(readUint64());
}
uint64_t ByteArray::readUint64() {
    uint64_t result = 0;
    for(int i = 0; i < 64; i += 7) {
        uint8_t b;
        readFixInt(b);
        if(b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return result;
}
float    ByteArray::readFloat() {
    uint32_t v;
    readFixInt(v);
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}
double   ByteArray::readDouble() {
    uint64_t v;
    readFixInt(v);
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}
std::string ByteArray::readFixLengthString() {
    size_t len;
    readFixInt(len);
    std::string res;
    res.resize(len);
    readFromArray(&res[0], len);
    return res;
}
std::string ByteArray::readUnifxLengthString() {
    uint64_t len = readUint64();
    std::string res;
    res.resize(len);
    readFromArray(&res[0], len);
    return res;
}

void ByteArray::clear() {
    resetPos(0);
    m_eof = 0;
}

void ByteArray::getWriteBuffers(std::vector<iovec>& buffer, size_t buffLen) {
    auto prePos = getPos();
    while(buffLen) {
        size_t curNodePos = m_position % m_baseSize;
        size_t curleftSize = m_baseSize - curNodePos;
        iovec vec;
        if (curleftSize >= buffLen) {
            vec.iov_len = buffLen;
            vec.iov_base = m_cur->ptr + curNodePos;
            buffer.push_back(vec);
            m_position += buffLen;
            buffLen = 0;
        } else {
            vec.iov_len = curleftSize;
            vec.iov_base = m_cur->ptr + curNodePos;
            buffer.push_back(vec);
            m_position += curleftSize;
            buffLen -= curleftSize;
        }
        if (m_position >= m_eof) m_eof = m_position;
        if (m_position && m_position % m_baseSize == 0) {
            if (!m_cur->next) {
                resize(buffLen + 1);
            }
            m_cur = m_cur->next;
        }
    }
    resetPos(prePos);
}

std::string ByteArray::toHexString() {
    std::string str = toString();
    std::stringstream ss;

    for(size_t i = 0; i < str.size(); ++i) {
        if(i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
           << (int)(uint8_t)str[i] << " ";
    }

    return ss.str();
}

}