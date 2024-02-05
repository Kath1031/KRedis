cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 加载子目录
add_subdirectory(./src)
# add_subdirectory(./test)