# C++ 并发读书笔记

## 线程的基础操作

### 线程的创建

线程在 `std::thread` **对象创建时启动**，通常使用的是无参数无返回的函数。这种函数在执行完毕，
线程也就结束了。

线程的创建支持所有的函数对象。
1. 函数指针
2. 类对象
3. lambda表达式

值得注意的点是，当把函数对象传入到线程构造函数中时，需要避免“最令人头痛的语法解析”(C++’s most
vexing parse, 中文简介)。如果你传递了一个临时变量，而不是一个命名的变量。C++编译器会将其解析为函
数声明，而不是类型对象的定义。
```c++
class A {
public:
    void operator()() {
        std::cout << 'A';
    }
};
int main()
{
    A a;
    std::thread t(A());
    t.join();
    return 0;
}
报错
member reference base type 'std::thread (A (*)())'
这说明A()被解析为了函数指针
C++函数声明的集中格式
int f(double d)； //声明接受一个double参数d，返回值为int类型的函数  
int f(double (d))；//效果一样，参数名外的括号会被忽略  
int f(double)；//直接省略参数名 
int g(double (*pf)())； //声明返回值为int类型的函数，接受一个返回类型为double无参数的函数指针pf 
int g(double pf())；//效果一样，pf是隐式函数指针  
int g(double ())；//直接省略参数名 
```

线程启动后是要等待线程结束，还是让其自主运行。当 `std::thread` 对象销毁之前还没有做出决定，程序就会
终止( `std::thread` 的析构函数会调用 `std::terminate()` )。因此，**即便是有异常存在，也需要确保线程能够正确
汇入(`joined`)或分离(`detached`)。**

需要注意的是：**如果不等待线程汇入 ，就必须保证线程结束之前，访问数据的有效性。**

### 参数的移动
参数的传递有以下几个值得注意的点

1. 向可调用对象或函数传递参数很简单，只需要将这些参数作为 `std::thread` 构造函数的附加
   参数即可。需要注意的是，这些参数会拷贝至新线程的内存空间中(同临时变量一样)。即使函数中的参数是引
   用的形式，拷贝操作也会执行。
    ```c++
    在这个拷贝当中，const char*会被转换为std::string 以 右值 的形式拷贝
    因此如何接受的参数不是const的引用会发生编译错误，需要传递引用需要使用std::ref()显示的传递
    void f(int i, std::string const& s);
    std::thread t(f, 3, "hello");
    ```
2. 除此之外，传动态变量的指针可能会有未定义的行为。
   ```cpp
   void f(int i,std::string const& s);
   void not_oops(int some_param)
   {
   char buffer[1024];
   sprintf(buffer,"%i",some_param);
   std::thread t(f,3,buffer); //这样的写法可能导致buffer在被转化为std::string的过程前被释放导致未定义行为
   std::thread t(f,3,std::string(buffer)); // 使用std::string，避免悬空指针
   t.detach();
   }
   ```
3. thread支持类似[bind](#std::bind实现机制) 将成员方法绑定到对象指针上
   ```c++
   class X
   {
   public:
   void do_lengthy_work();
   };
   X my_x;
   std::thread t(&X::do_lengthy_work, &my_x);
   ```
### 线程所有权的转换


### 确认线程的数量
`std::thread::hardware_concurrency()`在新版C++中非常有用，其会返回并发线程的数量。例如，多核系统中，
返回值可以是CPU核芯的数量。返回值也仅仅是一个标识，当无法获取时，函数返回0。

这段代码值得学习的地方： 1.如何确定并行计算的核心数 2.如何更好的写出模块化的代码(更好的理由迭代器和内置的函数)

```c++
template<typename Iterator,typename T>
struct accumulate_block
{
    void operator()(Iterator first,Iterator last,T& result)
    {
        result=std::accumulate(first,last,result);
    }
};
template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
    unsigned long const length=std::distance(first,last);
    if(!length) // 1
        return init;
    unsigned long const min_per_thread=25;
    unsigned long const max_threads=
            (length+min_per_thread-1)/min_per_thread; // 2
    unsigned long const hardware_threads=
            std::thread::hardware_concurrency();
    unsigned long const num_threads= // 3
            std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size=length/num_threads; // 4
    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads-1); // 5
    Iterator block_start=first;
    for(unsigned long i=0; i < (num_threads-1); ++i)
    {
       Iterator block_end=block_start;
       std::advance(block_end,block_size); // 6
       threads[i]=std::thread( // 7
          accumulate_block<Iterator,T>(),
          block_start,block_end,std::ref(results[i]));
          block_start=block_end; // 8
    }
    accumulate_block<Iterator,T>()(
    block_start,last,results[num_threads-1]); // 9
    for (auto& entry : threads)
    entry.join(); // 10
    return std::accumulate(results.begin(),results.end(),init); // 11
}
```

## 附录

### std::bind实现机制