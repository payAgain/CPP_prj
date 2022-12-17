#include "file_util.h"
#include "iostream"
#include "log.h"

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

    file.open(DEFAULT_LOG_PATH, dreamer::get_Today(), std::ios::app);
    char *p = "Zoey \n";
    file.write(p, strlen(p));
    std::cout << file.is_open() << std::endl;
}