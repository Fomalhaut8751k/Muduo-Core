cmake_minimum_required(VERSION 3.0)
project(mymuduo)

# 设置最终编译成的so动态库，最终存储的路径lib
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 配置编译选项，并且启动c++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 配置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 加载(源文件的)子目录
add_subdirectory(src)  # 通过三个CMakeLists.txt从当前目录./加载到./src/server

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)