#ifndef DREAMER_FILE_UTIL_H
#define DREAMER_FILE_UTIL_H


#include "fstream"
#include "string"

/**
 * 文件工具设计
 * 
 * 将文件操作封装在类中，通过对象的生命周期管理文件流
 * 对于该对象来说，对象的流只能移动，不能复制。
 * 
 * 应该包含的基础操作
 * 1. 创建文件
 * 2. 重载 >> & <<
 * 
*/

namespace dreamer {

/**
 * 对于一个文件的操作，在一个类中产生，并自动销毁。
*/
class FileOperation {
public:
    FileOperation() = default;
    FileOperation(std::string path, std::ios_base::openmode mode)
                : m_fs(path, mode) {}
    ~FileOperation();
    FileOperation(const FileOperation& obj) = delete; // 禁用拷贝构造函数
    FileOperation& operator=(const FileOperation& obj) = delete; // 禁用
    // 实现移动语义
    FileOperation(FileOperation &&obj);
    FileOperation& operator=(FileOperation &&obj) {
        if (m_fs.is_open()) {
            m_fs.close();
        }
        m_fs = std::move(obj.m_fs);
        m_path = std::move(obj.m_path);
        return *this;
    }
    int open(std::string path, std::ios_base::openmode mode);
    bool is_open();
    FileOperation& operator>>(std::string os) {
        m_fs >> os;
        return *this;
    }
    FileOperation& operator<<(std::string os) {
        m_fs << os;
        return *this;
    }
    std::fstream& get_stream() { return m_fs; }
private:
    std::fstream m_fs;
    std::string m_path;
};


}



#endif