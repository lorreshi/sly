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
        sly/thread.h
        sly/thread.cc
        sly/sly.h
        sly/macro.h
        sly/fiber.h
        sly/fiber.cc
        sly/scheduler.h
        sly/scheduler.cc
        sly/scheduler.cc
        sly/iomanager.h
        sly/iomanager.cc
        sly/timer.h
        sly/timer.cc
        sly/hook.h
        sly/hook.cc
        sly/fd_manager.h
        sly/fd_manager.cc
        sly/address.h
        sly/address.cc
        sly/socket.h
        sly/socket.cc
        sly/noncopyable.h
        sly/bytearray.h
        sly/bytearray.cc
        sly/http/http.h
        sly/http/http.cc
        sly/http/http11_common.h
        sly/http/http11_parser.h
        sly/http/httpclient_parser.h
        sly/http/http11_parser.cc
        sly/http/httpclient_parser.cc
        sly/http/http_parser.h
        sly/http/http_parser.cc)

# 把set中的文件生成动态库
add_library(sly SHARED ${LIB_SRC})

set(LIB_LIB
        sly
        pthread
        dl
        ${YAMLCPP})


#生成测试文件 测试日志
add_executable(test tests/test.cc)
add_dependencies(test sly)
#连接
target_link_libraries(test ${LIB_LIB})


#生成测试文件 测试config配置
add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sly)
#连接
#target_link_libraries(test_config sly -L/home/sly/CLionProjects/sly/cmake-build-debug/lib -lyaml-cpp)
target_link_libraries(test_config sly ${LIB_LIB})


#生成测试文件 测试Thread_test
add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread sly)
#连接
target_link_libraries(test_thread  sly ${LIB_LIB})


#生成测试文件 测试test_util
add_executable(test_util tests/test_util.cc)
add_dependencies(test_util sly)
#连接
target_link_libraries(test_util sly ${LIB_LIB})

#生成测试文件 测试test_fiber
add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber sly)
#连接
target_link_libraries(test_fiber sly ${LIB_LIB})

#生成测试文件 测试test_scheduler
add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler sly)
#连接
target_link_libraries(test_scheduler sly ${LIB_LIB})

#生成测试文件 测试test_iomanager
add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager sly)
#连接
target_link_libraries(test_iomanager sly ${LIB_LIB})

#生成测试文件 测试test_hook
add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook sly)
#连接
target_link_libraries(test_hook sly ${LIB_LIB})

#生成测试文件 测试test_address
add_executable(test_address tests/test_address.cc)
add_dependencies(test_address sly)
#连接
target_link_libraries(test_address sly ${LIB_LIB})

#生成测试文件 测试test_socket
add_executable(test_socket tests/test_socket.cc)
add_dependencies(test_socket sly)
#连接
target_link_libraries(test_socket sly ${LIB_LIB})

#生成测试文件 测试test_bytearray
add_executable(test_bytearray tests/test_bytearray.cc)
add_dependencies(test_bytearray sly)
#连接
target_link_libraries(test_bytearray sly ${LIB_LIB})

#生成测试文件 测试test_http
add_executable(test_http tests/test_http.cc)
add_dependencies(test_http sly)
#连接
target_link_libraries(test_http sly ${LIB_LIB})

#生成测试文件 测试test_parser
add_executable(test_parser tests/test_parser.cc)
add_dependencies(test_parser sly)
#连接
target_link_libraries(test_parser sly ${LIB_LIB})

#可以将可执行文件输出到二进制目录下的 bin 子目录中
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#${PROJECT_BINARY_DIR} 同样表示当前项目的二进制目录。通过将 ${PROJECT_BINARY_DIR}/lib
#赋值给 LIBRARY_OUTPUT_PATH，可以将生成的库文件输出到二进制目录下的 lib 子目录中。
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


