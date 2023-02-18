#include "file_util.h"
#include <string.h>
#include "log.h"
namespace dreamer {

typedef struct stat STATE;

std::string get_Today() {
    time_t time1 = time(nullptr);
    tm* t = localtime(&time1);
    char s[40];
    strftime(s, sizeof(s), FILE_DAY_PATTERN, t);
    return s;
}

int get_type(const std::string &path) {
    STATE st1;
    if (stat(path.c_str(), &st1) == 0) {
        if (st1.st_mode & S_IFDIR) {
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << path << " is a dir" ;
            return FileState::DIR;
        } else if (st1.st_mode & S_IFREG) {
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << path << " is a file" ;
            return FileState::FILE;
        } else {
            D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << path << " is not a file and a dir" ;
            return FileState::OTHER;
        }
    } else {
        D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << path << " do not exit" ;
        return FileState::INVALID;
    }
}

std::vector<std::string> get_files(std::string path, filter f, compar cmp) {
    struct dirent **namelist;
    auto ret = scandir(path.c_str(), &namelist, f, cmp);
    std::vector<std::string> tmp;
    if (ret >= 0) {
        D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "目录打开成功： namelist长度为" << ret;
        for(int i = 0; i < ret; i++) {
            tmp.emplace_back(path + '/' + namelist[i]->d_name);
            free(namelist[i]);
        }
        free(namelist);
    } else {
        D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "open dir fail or no enough memory";
    }
    return tmp;
}

size_t GetFileSize(const std::string& filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


#if __cplusplus >= 201703L
std::vector<std::string> get_files(std::string path) {
    std::vector<std::string> res;
    for (auto& i : std::filesystem::directory_iterator(path)) {
        res.push_back(i.path().string());
    }
    return res;
}
#else
std::vector<std::string> get_files(std::string path) {
return get_files(path, [](const struct dirent * t){ return 1; });
}
#endif

// 实现FileOperation
FileOperation::~FileOperation() {
    if (is_open())
        m_fs.close();
}
FileOperation::FileOperation(FileOperation &&obj)  noexcept {
    if (m_fs.is_open()) {
        m_fs.close();
    }
    m_fs = std::move(obj.m_fs);
}
int FileOperation::open(const std::string& path, std::ios_base::openmode mode) {
    if (m_fs.is_open()) {
        D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "filestream 已经打开， 请使用reopen重新打开" ;
        return -1;
    }
    m_fs.open(path, mode);
    if (m_fs.is_open()) {
        m_path = path;
        return 0;
    }
    D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << "filestream 打开文件失败" << strerror(errno);
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
            D_SLOG_WARN(DREAMER_SYSTEM_LOGGER()) << (file_path + "创建文件夹失败! \n");
            return -1;
        }
    }
    return open(file_path + file_name, mode);
}

void FileOperation::write(char *buf, std::streamsize size) {
    m_fs.write(buf, size);
}

void FileOperation::write(const std::string &str) {
    m_fs << str;
}

void FileOperation::read(char *buf, std::streamsize size) {
    m_fs.read(buf, size);
}


}
