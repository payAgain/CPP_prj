# 工具类技术文档

## 线程相关api

### 获取线程id

```cpp
#include <thread>
std::this_thread::get_id()
```
这种方式获取的是一个结构体，难以使用

```cpp
#include <unistd.h>
#include <sys/syscall.h>
static_cast<pid_t>(::syscall(SYS_gettid));
```
使用系统调用的方式获取线程的id

### 修改与获取线程的name

```c++
int pthread_getname_np(pthread_t, char *, size_t);
#if defined(__linux__)
pthread_setname_np(pthread_self(), "ThreadName");
#elif __APPLE__
pthread_setname_np("ThreadName");
#endif
```

## 附录

### 编译器提示性语法

#### [[nodiscard]]
```c++
struct SomeInts
{
   [[nodiscard]] bool empty();
   void push_back(int);
   //etc
};

void random_fill(SomeInts & container,
      int min, int max, int count)
{
   container.empty(); // empty it first
   for (int num : gen_rand(min, max, count))
      container.push_back(num);
}
```
加上[[nodiscard]]后，编译器发现empty()函数被调用时并没有使用它的返回值，则会报一个诸如warning: ignoring return value of 'bool empty()'的警告。

### 跨平台问题

对于同样的函数 Linux和apple的api接口是不同的，在这些平台之间编译时，可以使用下面的方式区分。
```c++
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         // iOS, tvOS, or watchOS Simulator
    #elif TARGET_OS_MACCATALYST
         // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
    #elif TARGET_OS_MAC
        // Other kinds of Apple platforms
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __ANDROID__
    // Below __linux__ check should be enough to handle Android,
    // but something may be unique to Android.
#elif __linux__
    // linux
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif
```