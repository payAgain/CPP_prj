# 参考

[C++日志开源框架1](https://gitbook.cn/gitchat/column/5b2c5b29072e851cae4299f3/topic/5b2c633e072e851cae42a1aa#:~:text=C%2B%2B%20%E4%B8%AD%E7%9A%84%E6%97%A5%E5%BF%97%E6%A1%86%E6%9E%B6%E6%9C%89%E5%BE%88%E5%A4%9A%EF%BC%8C%E5%85%B6%E4%B8%AD%E6%AF%94%E8%BE%83%E8%91%97%E5%90%8D%E7%9A%84%E6%9C%89%EF%BC%9A)

[开源框架2](https://www.zhihu.com/question/37640953)

[如何设计一个日志系统](https://www.zhihu.com/question/293863155)

[大佬的库](http://www.templog.org/)

[log4j解析](https://www.cnblogs.com/binarylei/p/10788315.html)

来自大佬的迭代建议
1. 做一个前后端不分离的，确定好Logger的调用格式。
2. 分离前后端，确定中间数据的格式。
3. 封装后端，确定后端的接口。
4. 实现异步后端

## 日志框架类图
![类图](https://s2.ax1x.com/2019/06/17/VHj5Of.png)

## 设计思路
每一个日志都是一个LogEvent事件，在LogEvent中包含当前日志使用的日志器(Q: 日志是否有必要像LogAppender一样被多个日志器输出？)和产生日志的[信息](#logevent-日志事件)，而LogEventWraper用于包装LogEvent，利用包装类的生命周期对LogEvent进行管理。

对于Logger来说，Logger是日志输出配置的一个具体的对象，可以通过YAML来对Logger进行一个配置(读取YAML文件的配置)，而LogAppender和LogFormatter是日志配置的抽象对象。

**Sylar中有一个非常巧妙的设计，Sylar的日志对象通过LogWrap进行包装，利用Wrap的析构函数对Event进行管理，在释放内存的同时将日志进行输出，不同于Java的内存管理机制，要是与Log4j的框架流程相同，那么日志对象的管理需要手动进行**

日志执行流程:

Logger ----> LogEvent ---> LogAppender ---> LogFilter ---> LogFormatter

append() ---> filter() ---> format()

遇到的设计问题：
- Pattern如何设计实现子类解析的问题
  > 最后选择仿造Log4j设计思路，对于简单的文本替换，直接定义一个BasicParser，对于日期这种需要针对性格式化的定义一个相应的实现类DataParser
- 采用log4j模式的设计导致了对__FILE__和__LINE__的调用时机必须在写日志的地方，那么单独对info,error进行封装显得很多余


## 具体类设计

### LogEvent 日志事件
LogEvent是日志产生的地方，因此在产生的时候需要保存记录多个数据。
```cpp
    /// 文件名
    const char* m_file = nullptr;
    /// 行号
    int32_t m_line = 0;
    /// 程序启动开始到现在的毫秒数
    uint32_t m_elapse = 0;
    /// 线程ID
    uint32_t m_threadId = 0;
    /// 协程ID
    uint32_t m_fiberId = 0;
    /// 时间戳
    uint64_t m_time = 0;
    /// 线程名称
    std::string m_threadName;
    /// 日志内容流
    std::stringstream m_ss;
    /// 日志器
    std::shared_ptr<Logger> m_logger;
    /// 日志等级
    LogLevel::Level m_level;
```

### Logger 日志器



### LogAppender 日志输出地

目前暂定文件和标准输出

### LogFormatter 日志格式化器

对于格式化输出器，我更偏向于使用log4j的设计思路，也就是Layout而不是封装FormatItem进行遍历(Salay中的实现方式)。
实现一个SimpleLogFormatter, ParternLogFormatter

## 学到的东西

### 利用宏和Function模板生成静态map
```c++
    static std::map<std::string, std::function<ParserItem::ptr(const std::string& pattern)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& pattern) { return Parser::ptr(new C(fmt));}}
            XX(p, LevelParser),             //p:日志级别
            XX(r, ElapseParser),            //r:累计毫秒数
            XX(c, NameParser),              //c:日志名称
            XX(t, ThreadIdParser),          //t:线程id
            XX(n, NewLineParser),           //n:换行
            XX(d, DateTimeParser),          //d:时间
            XX(f, FileParser),          //f:文件名
            XX(l, LineParser),              //l:行号
            XX(T, TabParser),               //T:Tab
            XX(F, FiberIdParser),           //F:协程id
            XX(N, ThreadNameParser),        //N:线程名称
#undef XX
    };
```