# 工具类技术文档







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