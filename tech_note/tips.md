# 一些比较实用的用法


## std::string
1. find_first_not_of  
```c++
return :
The position of the first character that does not match.
If no such characters are found, the function returns string::npos.
// string::find_first_not_of
#include <iostream>       // std::cout
#include <string>         // std::string
#include <cstddef>        // std::size_t

int main ()
{
  std::string str ("look for non-alphabetic characters...");

  std::size_t found = str.find_first_not_of("abcdefghijklmnopqrstuvwxyz ");

  if (found!=std::string::npos)
  {
    std::cout << "The first non-alphabetic character is " << str[found];
    std::cout << " at position " << found << '\n';
  }

  return 0;
}
```
2. std::transform()
```c++
    std::string a = "ABD";
    std::transform(a.begin(), a.end() , a.begin(), ::tolower); // :: indicate global space
    D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << a << std::endl;
```

## C++ Language

### C++ 初始化顺序
因在线程是出现同步问题而产生的知识点总结
#### 构造函数
在构造函数中，列表初始化一定是限于构造函数执行，在写线程的类时，出现过因为在构造函数值
实现列表初始化导致线程先执行的情况下未赋值的情况发生 需要注意

### const 用法总结

1. 修饰变量
   - `const int a = 10` a的值不能被修改
   - `const int* a = &b or int const* a = &b` 不能通过dereference修改b的值
   - `int* const a = &b` a的指向固定，只能指向b
   - `const int& a = b` 修饰引用 a 固定指向 b 
2. 修饰函数
   - 修饰参数  `void f(const int a)`; 与修饰变量一致
   - 修饰返回值  `const int f()`; 返回值不能作为左值使用
   - 修饰方法 void f() const 函数不会修改成员变量的值 除了被`mutable`修饰的变量
### 虚函数的定义
对于虚函数来说，虚函数的定义是必须实现的。 纯虚函数是不需要定义的，但是纯虚函数是可以被定义的，只能在类外进行定义。
```c++
class A {
    virtual void func(); // 需要实现
    virtual void func_a() = 0; // 纯虚函数
};

//纯虚函数的类外定义
void A::func_a() {
    
}
```

### 抽象类在什么时候可以被作为返回值？

### 根据传递的参数实例化不同的类
直接参与模版类，再进行强制类型转换
```c++
class A {
public:
A() {}
virtual ~A() = default;
virtual void get_v() = 0;
};

template<class T>
class B : A {
public:
B(T v) : m_v(v){}
void get_v() {
cout << m_v << endl;
}
private:
T m_v;
};

template<class T, class V>
A* get_instance(V v) {
A* a = (A *)new T(v);
return a;
}

int main() {
auto c = get_instance<B<int>, int>(10);
c->get_v();
}
```
也可以利用回调函数进行实现，麻烦，因新冠脑子烧坏了写出来的东西
```c++
template<class Func, class V>
ConfigVarBase::ptr look_up(Func func, std::string name, std::string desc, V v) {
    return func(name, desc, v);
}

static ConfigVarBase::ptr get_instance(std::string name, std::string desc, V v) {
std::shared_ptr<ConfigVarBase> res;
//        res.reset((ConfigVarBase *)new ConfigVar(name, desc, v));
res.reset(dynamic_cast<ConfigVarBase*>(new ConfigVar(name, desc, v)));
return res;
}

auto t = dreamer::ConfigMgr::getInstance()->look_up(dreamer::ConfigVar<int>::get_instance, "port", "port", 80);
D_SLOG_INFO(DREAMER_SYSTEM_LOGGER()) << t->to_string() << std::endl;
```


### dynamic_cast 和 dynamic_pointer_cast 是不一样的!
```c++
    std::dynamic_pointer_cast 属于std namespcae 只能用于转换shared_ptr
    dynamic_cast 是全局的动态类型转换
```

## Design Pattern

### Singleton
```c++
template<class T>
class Singleton{
public:
    static T* getInstance() {
        static T t;
        return &t;
    }
};

template<class T>
class SharedSingleton{
public:
    static std::shared_ptr<T*> getInstance() {
        static std::shared_ptr<T*> t{new T()};
        return t;
    }
};
```
## 利用宏来实现代码的兼容性
[不用版本的宏定义](https://blog.csdn.net/guoqx/article/details/121043769)
```c++
#if __cplusplus >= 201703L
std::list<std::string> get_files(std::string& path) {
    std::list<std::string> res;
    for (auto& cnt : std::filesystem::directory_iterator(path)) {
		res.push_back(cnt.path().string());
	}
    return res;
}
#else
std::list<std::string> get_files(std::string& path) {
    return get_files(path, [](const struct dirent * t){ return 0; }, alphasort);
}
#endif
```

## C++编译中的问题
1. Mac更新，项目炸了
   - 解决方法: 删除build内的缓存，重新加载

## 编码的细节
1. 对外暴露的宏 Marco 不要放在命名空间内