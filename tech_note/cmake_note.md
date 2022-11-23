# CMake 记录

主要学习关键字的用法和记录模版


## CMake关键字

### CMake变量

#### BUILD_SHARED_LIBS
是 add_library() 的一个全局标志，当`add_library()`没有指定 [STATIC | SHARED | MODULE] 是哪一个时，可以决定为生成动态库还是静态库。

Typical values：
- ON: 让 add_library() 生成 .dll 动态库，对应 SHARED
- OFF: 让 add_library() 生成 .lib 静态库，对应 STATIC ；默认值

#### CMAKE_BUILD_TYPE
指定 生成产物 的构建类型。

Typical values：
- `Debug`：详细调试信息
- `Release`：无调试信息
- `RelWithDebInfo`：带有调试信息的 Release，依旧可能又略微优化
- `MinSizeRel`：没使用过

### CMake关键字

#### [string](https://blog.csdn.net/m0_57845572/article/details/118520561)
```cmake
Search and Replace
  string(FIND <string> <substring> <out-var> [...])
  string(REPLACE <match-string> <replace-string> <out-var> <input>...)
  string(REGEX MATCH <match-regex> <out-var> <input>...)
  string(REGEX MATCHALL <match-regex> <out-var> <input>...)
  string(REGEX REPLACE <match-regex> <replace-expr> <out-var> <input>...)

Manipulation
  string(APPEND <string-var> [<input>...])
  string(PREPEND <string-var> [<input>...])
  string(CONCAT <out-var> [<input>...])
  string(JOIN <glue> <out-var> [<input>...])
  string(TOLOWER <string> <out-var>)
  string(TOUPPER <string> <out-var>)
  string(LENGTH <string> <out-var>)
  string(SUBSTRING <string> <begin> <length> <out-var>)
  string(STRIP <string> <out-var>)
  string(GENEX_STRIP <string> <out-var>)
  string(REPEAT <string> <count> <out-var>)

Comparison
  string(COMPARE <op> <string1> <string2> <out-var>)

Hashing
  string(<HASH> <out-var> <input>)

Generation
  string(ASCII <number>... <out-var>)
  string(HEX <string> <out-var>)
  string(CONFIGURE <string> <out-var> [...])
  string(MAKE_C_IDENTIFIER <string> <out-var>)
  string(RANDOM [<option>...] <out-var>)
  string(TIMESTAMP <out-var> [<format string>] [UTC])
  string(UUID <out-var> ...)

JSON
  string(JSON <out-var> [ERROR_VARIABLE <error-var>]
         {GET | TYPE | LENGTH | REMOVE}
         <json-string> <member|index> [<member|index> ...])
  string(JSON <out-var> [ERROR_VARIABLE <error-var>]
         MEMBER <json-string>
         [<member|index> ...] <index>)
  string(JSON <out-var> [ERROR_VARIABLE <error-var>]
         SET <json-string>
         <member|index> [<member|index> ...] <value>)
  string(JSON <out-var> [ERROR_VARIABLE <error-var>]
         EQUAL <json-string1> <json-string2>)

```

#### [file](https://juejin.cn/post/6844903634170298382) 文件操作

#### add_dependencies
在项目中通常会遇见这样的情况：（例如一个项目中有：main，libhello.a， libworld.a），当项目过小的时候，
编译顺序是*.a，然后是main，但是当一个项目的文件过于庞大，就会导致编译的顺序不会按照主CMAKE的add_subdirectory引入的先后顺序，
为了解决这一问题，就需要使用add_dependencies进行依赖指定。

## Cmake模版

项目根目录
```cmake
cmake_minimum_required(VERSION 3.23)

project(dreamer VERSION 0.0.1 LANGUAGES CXX)

#set(CMAKE_CXX_FLAGS "") 设置编译的选项
set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("Setting default build type to Debug")
endif()


include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(./test)
add_subdirectory(./src)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/lib)
```

测试
```cmake
#file(GLOB_RECURSE all_srcs ./*.cpp) 不推荐使用 效率低
aux_source_directory(./ all_srcs)
foreach(v ${all_srcs})
#    message(${v})
#    string(REGEX MATCH "src/.*" relative_path ${v})
#    message(${relative_path})
    string(REGEX REPLACE ".//" "" target_name ${v})
    string(REGEX REPLACE ".cpp" "" target_name ${target_name})
#    message(STATUS ${target_name})
    add_executable(${target_name} ${v})
    target_link_libraries(${target_name} dreamer_lib)
endforeach()



```

src
```cmake
aux_source_directory(./log LIB_SRC)


#include_directories()
add_library(dreamer_lib SHARED ${LIB_SRC})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} dreamer_lib)

#add_definitions(${PROJECT_NAME} dreamer_lib)
```