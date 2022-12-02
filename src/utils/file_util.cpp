#include "file_util.h"


namespace dreamer {


// 实现FileOperation
FileOperation::~FileOperation() {
    m_fs.close();
}
FileOperation::FileOperation(FileOperation &&obj) {
    if (m_fs.is_open()) {
        m_fs.close();
    }
    m_fs = std::move(obj.m_fs);
    m_path = std::move(obj.m_path);
}
int FileOperation::open(std::string path, std::ios_base::openmode mode) {
    if (m_fs.is_open()) {
        return -1;
    }
    m_fs.open(path, mode);
    if (m_fs.is_open()) {
        m_path = path;
        return 0;
    }
    return -2;
}
bool FileOperation::is_open() {
    return m_fs.is_open();
}


}
