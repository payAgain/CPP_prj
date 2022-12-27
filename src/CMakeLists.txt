aux_source_directory(./log LIB_SRC)
aux_source_directory(./utils LIB_SRC)
aux_source_directory(./config LIB_SRC)


#include_directories()
add_library(dreamer_lib SHARED ${LIB_SRC})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} dreamer_lib pthread)

#add_definitions(${PROJECT_NAME} dreamer_lib)