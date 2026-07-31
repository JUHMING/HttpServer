cmake_minimum_required(VERSION 3.22)

project(HTTPServer
        VERSION 1.0
        LANGUAGES CXX)


# =========================
# C++ 标准
# =========================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# =========================
# 编译选项
# =========================

add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -O2
)


# =========================
# 源文件
# =========================

file(GLOB_RECURSE SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/*.cpp
)


# =========================
# 可执行文件
# =========================

add_executable(
        HTTPServer
        ${SRC_FILES}
        src/protocol/MessageCodec.h
        src/protocol/MessageCodec.cpp
        src/log/Logger.h
)


# =========================
# 头文件路径
# =========================

target_include_directories(
        HTTPServer
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)


# =========================
# Linux库
# =========================

target_link_libraries(
        HTTPServer
        PRIVATE
        pthread
)


# =========================
# Debug信息
# =========================

set(CMAKE_BUILD_TYPE Debug)