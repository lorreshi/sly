cmake_minimum_required(VERSION 3.10)
project(sly)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)
include_directories(/home/sly/CLionProjects/sly/yaml-cpp/include/)
link_directories(/home/sly/CLionProjects/sly/yaml-cpp/include/)

find_library(YAMLCPP libyaml-cpp.a)


set(LIB_SRC
        sly/log.cc
        sly/log.cc
        sly/util.h
        sly/util.cc
        sly/singleton.h
        sly/config.h
        sly/config.cc
        )

# 把set中的文件生成动态库
add_library(sly SHARED ${LIB_SRC})

##生成测试文件 测试日志
#add_executable(test ${LIB_SRC})
#add_dependencies(test sly)
##连接
#target_link_libraries(test sly)

#生成测试文件 测试config配置
add_executable(test_config tests/test_config.cc sly/log.cc)
add_dependencies(test_config sly)
#连接
target_link_libraries(test_config sly -L/home/sly/CLionProjects/sly/cmake-build-debug/lib -lyaml-cpp)

#可以将可执行文件输出到二进制目录下的 bin 子目录中
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#${PROJECT_BINARY_DIR} 同样表示当前项目的二进制目录。通过将 ${PROJECT_BINARY_DIR}/lib
#赋值给 LIBRARY_OUTPUT_PATH，可以将生成的库文件输出到二进制目录下的 lib 子目录中。
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


