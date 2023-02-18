aux_source_directory(./log LIB_SRC)
aux_source_directory(./utils LIB_SRC)
aux_source_directory(./config LIB_SRC)
aux_source_directory(./http LIB_SRC)
aux_source_directory(./server LIB_SRC)
aux_source_directory(./stream LIB_SRC)

#include_directories()
add_library(dreamer_lib SHARED ${LIB_SRC})
target_link_libraries(dreamer_lib yaml-cpp)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} yaml-cpp pthread dreamer_lib)

#add_definitions(${PROJECT_NAME} dreamer_lib)