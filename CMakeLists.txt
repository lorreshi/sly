cmake_minimum_required(VERSION 3.10)
project(sly)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC
        sly/log.cc

        )

add_library(sly SHARED ${LIB_SRC})

#生成测试文件
add_executable(test tests/test.cc)
add_dependencies(test sly)
#连接
target_link_libraries(test sly)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


