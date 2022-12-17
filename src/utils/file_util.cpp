#include "file_util.h"


namespace dreamer {

std::string get_Today() {
    time_t time1 = time(nullptr);
    tm* t = localtime(&time1);
    char s[40];
    strftime(s, sizeof(s), FILE_DAY_PATTERN, t);
    return s;
}

// 实现FileOperation
FileOperation::~FileOperation() {
    m_fs.close();
}
FileOperation::FileOperation(FileOperation &&obj)  noexcept {
    if (m_fs.is_open()) {
        m_fs.close();
    }
    m_fs = std::move(obj.m_fs);
    m_file_path = std::move(obj.m_file_path);
}
int FileOperation::open(const std::string& file_path, const std::string& file_name
        , std::ios_base::openmode mode) {
    if (m_fs.is_open()) {
        return -1;
    }
    m_fs.open(file_path + file_name, mode);
    if (m_fs.is_open()) {
        m_file_path = file_path;
        m_file_name = file_name;
        return 0;
    }
    return -2;
}
bool FileOperation::is_open() {
    return m_fs.is_open();
}
int FileOperation::open_and_create(const std::string& file_path, const std::string& file_name
        , std::ios_base::openmode mode) {
    // 该函数功能为确定文件或文件夹的访问权限，如果指定的访问权限有效，则函数返回0，否则返回-1
    // 文件不存在 创建
    if(access(file_path.c_str(), F_OK)) {
        // 该函数功能为建立一个新的目录，创建成功则返回0，否则返回-1
        if (mkdir(file_path.c_str(), S_IRWXG | S_IRWXO | S_IRWXU)) {
            perror((file_path + "创建文件夹失败! \n").c_str());
            return -1;
        }
    }
    return open(file_path, file_name, mode);
}

void FileOperation::write(char *buf, std::streamsize size) {
    m_fs.write(buf, size);
}

void FileOperation::write(const std::string &str) {
    m_fs << str;
}




}
