#file(GLOB_RECURSE all_srcs ./*.cpp)
aux_source_directory(./ all_srcs)
foreach(v ${all_srcs})
#    message(${v})
#    string(REGEX MATCH "src/.*" relative_path ${v})
#    message(${relative_path})
    string(REGEX REPLACE ".//" "" target_name ${v})
    string(REGEX REPLACE ".cpp" "" target_name ${target_name})
#    message(STATUS ${target_name})
    add_executable(${target_name} ${v} )
    target_link_libraries(${target_name} dreamer_lib pthread yaml-cpp)
endforeach()


