#ifndef __DREAMER_D_BYTEARRAY_H__
#define __DREAMER_D_BYTEARRAY_H__

#include <memory>
#include <stddef.h>

namespace dreamer {


class ByteArray {
public:
typedef std::shared_ptr<ByteArray> ptr;


/**
 * @brief 使用指定长度的内存块构造ByteArray
 * @param[in] base_size 内存块大小
 */
ByteArray(size_t base_size = 4096);
/**
 * @brief 操作外部已有内存,如果owner为false,不支持写操作
 * @param[in] data 内存指针
 * @param[in] size 数据大小
 * @param[in] owner 是否管理该内存
 */
// ByteArray(void* data, size_t size, bool owner = false);
/**
 * @brief 析构函数
 */
~ByteArray();

// class
void resize(size_t sizeNeed);
void resetPos(size_t p);
std::string toString();
size_t getPos() { return m_position; }
size_t getEOF() { return m_eof; }
void writeToArray(const void* buff, size_t buffSize);
void readFromArray(void* buff, size_t buffSize);
void writeToFile(const std::string& fileName, size_t buffSize);
void readFromFile(const std::string& fileName, size_t buffSize);
// void readAllFile(const std::string& fileName);

// write interface
template<class T>
void writeFixInt(T intV) {
    #if BYTE_ORDER != BIG_ENDIAN
        switch (sizeof(T)) {
        case 1:
            break;
        case 2:
            intV = htobe16(intV);
            break;
        case 4:
            intV = htobe32(intV);
            break;
        default:
            intV = htobe64(intV);
            break;
        }
    #endif
    writeToArray(&intV, sizeof(T));
}
void writeFloat(float v);
void writeDouble(double v);
void writeInt32(int32_t v);
void writeUint32(uint32_t v);
void writeInt64(int64_t v);
void writeUint64(uint64_t v);
void writeStringWithFixLength(const std::string& v);
void writeStringWithUnfixLength(const std::string& v);

// read
template<class T>
void readFixInt(T& res) {
    readFromArray(&res, sizeof(T));
        #if BYTE_ORDER != BIG_ENDIAN
        switch (sizeof(T)) {
        case 1:
            break;
        case 2:
            res = be16toh(res);
            break;
        case 4:
            res = be32toh(res);
            break;
        default:
            res = be64toh(res);
            break;
        }
    #endif 
}
// var int
int32_t  readInt32();
uint32_t readUint32();
int64_t  readInt64();
uint64_t readUint64();
float    readFloat();
double   readDouble();
std::string readFixLengthString();
std::string readUnifxLengthString();

private:
struct Node {
    /**
     * @brief 构造指定大小的内存块
     * @param[in] s 内存块字节数
     */
    Node(size_t s);
    /**
     * 无参构造函数
     */
    // Node();
    /**
     * 析构函数,释放内存
     */
    ~Node();
    /**
     * 释放内存
     */
    void freeNode();
    /// 内存块地址指针
    char* ptr;
    /// 下一个内存块地址
    Node* next;
    /// 内存块大小
    size_t size;
};


/// 内存块的大小
size_t m_baseSize;
/// 当前操作位置
size_t m_position;
/// end of file(byteArray)
size_t m_eof;
/// 当前的总容量
size_t m_capacity;
/// 当前数据的大小
size_t m_size;
/// 是否拥有数据的管理权限
bool m_owner;
/// 第一个内存块指针
Node* m_root;
/// 当前操作的内存块指针
Node* m_cur;

};

}




#endif