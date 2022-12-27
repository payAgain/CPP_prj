# 玩具

总体框架参考[sylar](https://github.com/sylar-yin/sylar)
细节部分根据现有开源框架进行修改，和C++20，ModernCPP等

## 日志设计

[详细设计文档](./tech_note/logimp.md)

## 配置系统

配置系统的设计思路和问题

值得思考的问题： 如何将json和yml进行整合，拓展性，既可以通过json配置，也可以通过yml。

### 问题：
1. 在实际的使用中，对象需要被封装在configVar中。
2. 对于自定义的类型，类型转换需要对应的类自己实现转化器。

### 细节
1. 对于配置的名称进行约束
2. 类型转换过程中的异常处理
3. 日志记录相关的信息
4. 使用智能指针对指针进行管理，防止内存泄露

### 设计思路：

#### ConfigVarBase
对ConfigVar进行抽象，为了保证相应的拓展性(JSON和YML的配置)
```c++
class ConfigVarBase {
    std::string name;
    std::string description;
    virtual std::string from_string();
    virtual std::string to_string();
    // 这个函数的设计很好 怎么样在指针向上转型之后依然能够获取模版的类型
    virtual std::string getTypeName() const = 0;
};
```
#### YMLConfigVar 设计思路
我认为根据sylar的设计思路，这个类命名为YML更为合适，更具有拓展性
````c++
template<class T, class From, Class To>
class YMLConfigVar ： ConfigVarBase{
    T value;
    // 实现相应的函数
};
````

#### ConfigManager --> Config
对每个ConfigVarBase进行管理。
本质就是对 `std::map<std::string, ConfigVarBase::ptr>` 这个数据结构实现一些方法，将其暴露出去
```c++
class ConfigMannager {
    static std::map<std::string, ConfigVarBase::ptr> configs;
};
```

## 工具设计
[线程库设计文档](./tech_note/thread.md)


## 引用的第三方库

## To-do
1. 日志框架线程同步问题。
2. FileAppender文件输入流的完善。 (已完成)
3. 日志管理类的设计。
4. 使用文件对日志进行配置。
5. 配置系统的框架搭建