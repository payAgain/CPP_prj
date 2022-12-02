#include "file_util.h"
#include "iostream"

int main() {
    dreamer::FileOperation file;
    int ret = file.open("/home/dym/Desktop/log", std::ios::out);

    // open_fail  can't use ~
    dreamer::FileOperation file1("~/Desktop/log1", std::ios::out);

    
}