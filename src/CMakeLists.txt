aux_source_directory(./ SRC)
add_executable(${PROJECT_NAME} ${SRC} ../include/log.h log.cpp)