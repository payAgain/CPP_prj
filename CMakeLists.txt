cmake_minimum_required(VERSION 3.23)

project(dreamer VERSION 0.0.1 LANGUAGES CXX)

#set(CMAKE_CXX_FLAGS "") 设置编译的选项
set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("Setting default build type to Debug")
endif()

find_package(Boost)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(yaml-cpp REQUIRED
        PATHS /opt/homebrew/Cellar/yaml-cpp/0.7.0
        NO_DEFAULT_PATH)
#find_library(YML yaml-cpp PATHS /opt/homebrew/Cellar/yaml-cpp/0.7.0)
if (yaml-cpp_FOUND)
#    message("yaml-cpp is found" ${YML})
    include_directories(${yaml-cpp_INCLUDE_DIRS})
endif()


include_directories(${PROJECT_SOURCE_DIR}/include)

add_definitions(-D_XOPEN_SOURCE)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/bin) # 要让这句话生效需要放在add_subdirectory()之前
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(./test)
add_subdirectory(./src)

