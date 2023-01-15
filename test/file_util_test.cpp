#include "file_util.h"
#include "iostream"
#include "log.h"

static int YMLFilter(const struct dirent *dir) {
    std::string p(dir->d_name);
    D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "before name :" << p;
    int pos = p.find_last_of('.');
    if (pos == std::string::npos) return 0;
    p = p.substr(pos, p.size());
    D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "after cut suffix :" << p;
    if (p == ".yml" && (dir->d_type & DT_REG)) return 1;
    else return 0;
}

int main() {
    dreamer::FileOperation file;
//    int ret = file.open("/home/dym/Desktop/log", std::ios::out);
//     open_fail  can't use ~
//    dreamer::FileOperation file1("~/Desktop/log1", std::ios::out);
//    未能创建成功但是也没报错 不能直接创建文件夹
//    dreamer::FileOperation file1(DEFAULT_LOG_PATH, dreamer::get_Today(), std::ios::out);
//    std::cout << file1.is_open() << std::endl;

//    file.open_and_create(DEFAULT_LOG_PATH, dreamer::get_Today(), std::ios::out);
//    std::cout << file.is_open() << std::endl;
//    std::cout << dreamer::get_Today() << std::endl;

//    file.open(DEFAULT_LOG_PATH, dreamer::get_Today(), std::ios::app);
//    char *p = "Zoey \n";
//    file.write(p, strlen(p));
//    std::cout << file.is_open() << std::endl;

    auto ret = dreamer::get_files("/Users/yimingd/desktop");
    for (auto i : ret) D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << i ;
    D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << "-----------------------------";
    auto res = dreamer::get_files("/Users/yimingd/desktop", YMLFilter);
    for (auto i : res) D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << i ;

}