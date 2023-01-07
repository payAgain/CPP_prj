#ifndef DREAMER_FILE_UTIL_H
#define DREAMER_FILE_UTIL_H


#include "fstream"
#include "string"
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "vector"
#include "filesystem"
#include "log.h"
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
 *  模式标志	描述
 * ios::app	追加模式。所有写入都追加到文件末尾。
 * ios::ate	文件打开后定位到文件末尾。
 * ios::in	打开文件用于读取。
 * ios::out	打开文件用于写入。
 * ios::trunc	如果该文件已经存在，其内容将在打开文件之前被截断，即把文件长度设为 0。
 *
 * #include <unistd.h>  access 函数
 * #include <sys/types.h>
 * #include <sys/stat.h> mkdir
 *
 * STATE:
 * DT_BLK                  This is a block device.
 * DT_CHR                  This is a character device.
 * DT_DIR      文件夹       This is a directory.
 * DT_FIFO                 This is a named pipe (FIFO).
 * DT_LNK      符号链接文件  This is a symbolic link.
 * DT_REG      文件         This is a regular file.
 * DT_SOCK     SOCKET链接   This is a UNIX domain socket.
 * DT_UNKNOWN              The file type is unknown.
*/

namespace dreamer {

#define FILE_DAY_PATTERN "%Y-%m-%d"

// 获取今天的日期
std::string get_Today();

// 获取文件的类型
int get_type(const std::string& path);

// 遍历文件夹
typedef int (*filter)(const struct dirent *);
typedef int (*compar)(const struct dirent **, const struct dirent **);
// 默认比较方法为alphasort
std::vector<std::string> get_files(std::string path, filter f, compar cmp = alphasort);
std::vector<std::string> get_files(std::string path);



enum FileState {
    FILE,
    DIR,
    OTHER,
    INVALID
};


/**
 * 对于一个文件的操作，在一个类中产生，并自动销毁。
*/
class FileOperation {
public:
    FileOperation() = default;
    FileOperation(std::string& path) : m_path(path), m_fs(path) {}
    ~FileOperation();
    FileOperation(const FileOperation& obj) = delete; // 禁用拷贝构造函数
    FileOperation& operator=(const FileOperation& obj) = delete; // 禁用
    FileOperation(FileOperation &&obj) noexcept ;
    FileOperation& operator=(FileOperation &&obj)  noexcept {
        if (m_fs.is_open()) {
            m_fs.close();
        }
        m_fs = std::move(obj.m_fs);
        return *this;
    }
    FileOperation& operator>>(std::string os) {
        m_fs >> os;
        return *this;
    }
    FileOperation& operator<<(const std::string& os) {
        m_fs << os;
        return *this;
    }

public:
    int open(const std::string& path, std::ios_base::openmode mode = std::ios::app);
    // 在已知需要创建文件夹目录的情况下打开文件
    int open_and_create(const std::string& file_path, const std::string& file_name
            , std::ios_base::openmode mode = std::ios::app);
    bool is_open();
    void write(char *buf, std::streamsize size);
    void write(const std::string& str);
    std::fstream& get_stream() { return m_fs; }
private:
    std::fstream m_fs;
    std::string m_path;

};


}



#endif